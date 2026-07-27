#!/usr/bin/env python3
"""Generate/check bridge's facility-based documentation pages.

See docs/adr/0014-documentation-site-architecture.md for the design
this implements: docs/pages/registry.yaml declares a set of
"facilities" (a conceptual division, e.g. "format-print"), each owning
a list of header files and a hand-written docs/pages/<name>.md page.
This script scaffolds a facility's page if it doesn't exist yet, and
regenerates machine-owned, marker-delimited blocks inside every
existing page (a "headers" list and a "symbols" table pulled from
Doxygen's own XML output (build/docs/xml), plus a prominent
cppreference header link) -- leaving hand-written prose between/around
them untouched. Every facility's hand-written "## Example" section is
also flattened (concatenated with its `example_headers`, inter-bridge
includes stripped) and compile-checked standalone under `-std=c++17`,
failing loudly if it doesn't compile and run cleanly -- this needs a
C++ compiler on `PATH` (the same one `./bridge probe` picks up),
including in CI's `check` mode.

Two modes:
  generate  Regenerate in place. Also fails (non-zero exit) if any
            include/**/*.hpp isn't covered by some facility's
            `headers` list -- run as part of `./bridge docs`.
  check     Same regeneration logic, but diffed against the committed
            page instead of written to it -- fails with the diff shown
            on any mismatch. Run in CI, not local `./bridge docs`.
"""
import argparse
import difflib
import re
import subprocess
import sys
import tempfile
import xml.etree.ElementTree as ET
from pathlib import Path

import yaml

REPO_ROOT = Path(__file__).resolve().parent.parent
REGISTRY_PATH = REPO_ROOT / "docs/pages/registry.yaml"
PAGES_DIR = REPO_ROOT / "docs/pages"
XML_DIR = REPO_ROOT / "build/docs/xml"
INCLUDE_DIR = REPO_ROOT / "include"

BLOCK_BEGIN = "<!-- BRIDGE-DOCS:BEGIN {tag} -->"
BLOCK_END = "<!-- BRIDGE-DOCS:END -->"

# Matches the promotion-chain pattern every Truss/Deck header in this
# codebase uses (docs/adr/0001-namespace-and-export-scheme.md):
# `using bridge::detail::...::name;` inside a header's `exports`
# namespace. Capturing just the final identifier is enough to know
# *which* names are actually promoted to the public surface, as
# opposed to every documented internal helper Doxygen also extracts
# from the same detail namespace.
PROMOTION_RE = re.compile(r"using\s+bridge::detail::[\w:]+::(\w+)\s*;")

# Matches a Truss/Deck detail-namespace prefix (bridge::detail::truss::
# cpp17::<facility> or bridge::detail::deck::cpp17::<facility>) --
# collect_symbols() uses this to decide whether a symbol's local name
# alone is enough to identify it (true here, since it's part of the
# tiered promotion chain) or whether its enclosing namespace has to
# stay part of the display name too (Rivets' flat, untiered
# namespaces, where the same local name is deliberately reused per
# Entity).
TIERED_NAMESPACE_RE = re.compile(r"^bridge::detail::(truss|deck)::cpp17::\w+(::exports)?$")


def load_registry():
    with open(REGISTRY_PATH) as f:
        data = yaml.safe_load(f)
    return data["facilities"]


def all_headers():
    return sorted(p.relative_to(REPO_ROOT).as_posix() for p in INCLUDE_DIR.rglob("*.hpp"))


def check_coverage(facilities):
    """Returns a list of header paths (relative to REPO_ROOT) that
    aren't assigned to any facility. Empty list means full coverage."""
    registered = set()
    for fac in facilities:
        registered.update(fac["headers"])
    return [h for h in all_headers() if h not in registered]


def promoted_names_for_header(header_rel):
    text = (REPO_ROOT / header_rel).read_text()
    return set(PROMOTION_RE.findall(text))


def strip_template_args(name):
    idx = name.find("<")
    return name[:idx].strip() if idx != -1 else name


def escape_markdown_plain(text):
    # Doxygen's Markdown pages get full autolink processing, unlike the
    # comments these briefs were originally lifted from: a bare `#word`
    # is parsed as an explicit link request ("#if-usable" -> tries to
    # resolve a symbol named "if-usable"), and a bare `<T>` is parsed as
    # an HTML tag start. Neither is a problem inside a `///` comment in
    # a .hpp file, only once the same text lands in a table cell here --
    # confirmed by hitting exactly these two Doxygen errors before
    # adding this escaping, not assumed. Only applied to text *outside*
    # a `<computeroutput>` code span -- inside one, `<`/`>`/`#` render
    # literally, same as any other Markdown code span.
    text = text.replace("|", "\\|")
    text = text.replace("<", "&lt;").replace(">", "&gt;")
    text = re.sub(r"#(?=\w)", r"\\#", text)
    return text


def render_inline(elem):
    """Reconstructs Markdown from a Doxygen `<para>`-like element's
    inline content: `<computeroutput>` (backtick-quoted code written in
    the original /// comment) becomes a code span, `<bold>`/`<emphasis>`
    become `**`/`*`, `<ref>` and anything else is flattened to its text.
    Doxygen's own XML already carries this structure -- this replaces
    the previous flatten-everything-via-itertext() approach, which threw
    it away and left escaped literal angle brackets in its place."""
    parts = []
    if elem.text:
        parts.append(escape_markdown_plain(elem.text))
    for child in elem:
        tag = child.tag
        if tag == "computeroutput":
            code = "".join(child.itertext()).replace("|", "\\|")
            parts.append(f"`{code}`")
        elif tag == "bold":
            parts.append(f"**{render_inline(child)}**")
        elif tag == "emphasis":
            parts.append(f"*{render_inline(child)}*")
        else:
            parts.append(render_inline(child))
        if child.tail:
            parts.append(escape_markdown_plain(child.tail))
    return "".join(parts)


def brief_text(elem):
    bd = elem.find("briefdescription")
    if bd is None:
        return ""
    text = " ".join(render_inline(para) for para in bd.findall("para"))
    return " ".join(text.split())


def collect_symbols(headers):
    """Returns a sorted list of {name, kind, brief} dicts for every
    documented compound/member Doxygen locates in one of `headers`.

    A header with at least one `using bridge::detail::...::x;`
    promotion line (the Truss/Deck tiering pattern) is filtered to only
    the names that pattern actually promotes -- otherwise every
    internal helper Doxygen documents in the same detail namespace
    would show up too. A header with *no* such lines (Rivets'
    detection headers, which deliberately have no detail/exports
    tiering at all, or a pure-macro header like diagnostics.hpp) is
    treated as fully public: nothing to filter.
    """
    if not XML_DIR.is_dir():
        return []

    promoted_by_abs_path = {
        str((REPO_ROOT / h).resolve()): promoted_names_for_header(h) for h in headers
    }
    header_abs = set(promoted_by_abs_path)

    def is_promoted(file_attr, name):
        promoted = promoted_by_abs_path.get(file_attr)
        if promoted is None:
            return False
        return (not promoted) or (name in promoted)

    results = {}  # display_name -> (kind, brief)

    def display_name(namespace_prefix, local):
        # Truss/Deck's tiered facilities (docs/adr/0001) legitimately
        # define the "same" public symbol twice -- Truss's real
        # definition, plus Deck's thin alias-selection re-declaration
        # (docs/adr/0014) -- and those should collapse to one row keyed
        # on the bare local name (see add_result below). Rivets'
        # facilities have no such tiering: `eq`/`ge`/`gt`/etc. are
        # deliberately reused *local* names across genuinely distinct
        # per-Entity namespaces (bridge::rivets::clang::eq is not the
        # same symbol as bridge::rivets::gcc::eq), so those need the
        # namespace kept in the display name or they'd wrongly collide.
        if TIERED_NAMESPACE_RE.match(namespace_prefix or ""):
            return local
        return f"{namespace_prefix}::{local}" if namespace_prefix else local

    def add_result(name, kind, brief):
        # A name can legitimately show up twice: Truss defines the
        # real thing (class/struct/function/enum), Deck's own
        # alias-selection header (docs/adr/0014) re-declares the
        # same name as a thin, separately-documented alias
        # ("typedef" in Doxygen's XML) pointing at whichever engine
        # is selected. Prefer the substantive definition over the
        # alias when both exist, regardless of XML processing order.
        existing = results.get(name)
        if existing is None or (existing[0] == "typedef" and kind != "typedef"):
            results[name] = (kind, brief)

    for xml_file in sorted(XML_DIR.glob("*.xml")):
        if xml_file.name == "index.xml":
            continue
        try:
            root = ET.parse(xml_file).getroot()
        except ET.ParseError:
            continue
        compounddef = root.find("compounddef")
        if compounddef is None:
            continue
        kind = compounddef.get("kind")

        if kind in ("class", "struct", "union"):
            loc = compounddef.find("location")
            if loc is None or loc.get("file") not in header_abs:
                continue
            raw_name = compounddef.findtext("compoundname") or ""
            prefix, _, local_raw = raw_name.rpartition("::")
            local = strip_template_args(local_raw)
            if not is_promoted(loc.get("file"), local):
                continue
            add_result(display_name(prefix, local), kind, brief_text(compounddef))

        elif kind in ("namespace", "file"):
            namespace_prefix = compounddef.findtext("compoundname") or ""
            for md in compounddef.iter("memberdef"):
                loc = md.find("location")
                if loc is None or loc.get("file") not in header_abs:
                    continue
                mkind = md.get("kind")
                name = md.findtext("name") or ""
                if mkind == "define":
                    # File-scope macros aren't namespaced, so the
                    # promotion pattern doesn't apply -- always public.
                    add_result(name, mkind, brief_text(md))
                    continue
                if not is_promoted(loc.get("file"), name):
                    continue
                add_result(display_name(namespace_prefix, name), mkind, brief_text(md))

    return [
        {"name": n, "kind": k, "brief": b}
        for n, (k, b) in sorted(results.items())
    ]


def render_headers_block(facility):
    lines = [f"- `{h}`" for h in facility["headers"]]
    return "\n".join(lines) + "\n"


def render_header_link_block(facility):
    """A prominent link to the facility's real C++ standard header
    (e.g. `<optional>`), placed right before the page's Synopsis --
    empty for facilities with no real std:: mirror (`header: null` in
    registry.yaml), so the marker stays uniform across every page
    without leaving stray markup on the ones that don't apply."""
    header = facility.get("header")
    if not header:
        return ""
    cppref = facility.get("cppreference")
    if cppref:
        return f"See [`{header}`]({cppref}) on cppreference.\n"
    return f"Mirrors `{header}`.\n"


def cppreference_stub(facility, symbol_name):
    """The compact `<header>::name` form shown in place of the full
    cppreference URL, so the generated table has room to breathe
    (especially on mobile) -- `header` is the facility's real C++
    standard header (docs/pages/registry.yaml's `header:` field), not
    derived from the cppreference URL's own path categorization, which
    often doesn't match the symbol's actual header (`std::format`'s
    cppreference path says "utility", but its real header is
    `<format>`)."""
    header = facility.get("header")
    if not header:
        return None
    # Rivets' untiered facilities keep their full C++ namespace in
    # `symbol_name` to stay unique in the table (docs/adr/0006) -- the
    # stub only wants the leaf identifier alongside the header, same as
    # the Truss/Deck tiered facilities where `symbol_name` is already
    # bare.
    leaf = symbol_name.rpartition("::")[2]
    return f"`{header}::{leaf}`"


def render_symbols_block(facility):
    symbols = collect_symbols(facility["headers"])
    cppref = facility.get("cppreference")
    header_row = "| Symbol | Kind | Brief |"
    sep_row = "|---|---|---|"
    if cppref:
        header_row += " cppreference |"
        sep_row += "---|"
    lines = [header_row, sep_row]
    for sym in symbols:
        name_cell = sym["name"].replace("|", "\\|")
        row = f"| `{name_cell}` | {sym['kind']} | {sym['brief']} |"
        if cppref:
            link_text = cppreference_stub(facility, sym["name"]) or cppref
            row += f" [{link_text}]({cppref}) |"
        lines.append(row)
    if not symbols:
        note_cols = 4 if cppref else 3
        lines.append("| _(no documented symbols found -- run `./bridge docs` first)_ |" + " |" * (note_cols - 1))
    return "\n".join(lines) + "\n"


EXAMPLE_HEADING_RE = re.compile(r"^## Example\s*$", re.MULTILINE)
CODE_FENCE_RE = re.compile(r"```cpp\n(.*?)\n```", re.DOTALL)
INTERNAL_INCLUDE_RE = re.compile(r"^#include <(?:rivets|truss|deck)/")


def extract_example_code(content, page_name):
    """Returns the code inside the first ```cpp fence following a
    "## Example" heading in `content`, or None if there's no such
    heading yet (a freshly-scaffolded page with no Example section
    written). Raises if the heading exists but no fenced code follows
    it -- that's an authoring mistake, not a legitimately-absent
    section."""
    heading_match = EXAMPLE_HEADING_RE.search(content)
    if heading_match is None:
        return None
    fence_match = CODE_FENCE_RE.search(content, heading_match.end())
    if fence_match is None:
        raise ValueError(f"{page_name}: '## Example' heading found but no ```cpp fenced code block after it")
    return fence_match.group(1)


def strip_internal_lines(text):
    """Drops every inter-bridge `#include <rivets|truss|deck/...>` and
    `#pragma once` line from `text` -- used when concatenating a
    facility's real headers into one flattened translation unit, where
    that content is already inlined and re-including it (or repeating
    the include guard) would either double-define everything or, for
    `#pragma once` specifically, just be dead weight in a single-TU
    context. Confirmed by hand-flattening `optional`'s real headers
    this way and compiling the result before writing this generally --
    a naive concatenation keeping those lines produced actual
    redefinition errors from the real files also being on the
    filesystem at their #include path."""
    kept = []
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == "#pragma once":
            continue
        if INTERNAL_INCLUDE_RE.match(stripped):
            continue
        kept.append(line)
    return "\n".join(kept)


def flatten_example(facility, example_code):
    """Concatenates `facility`'s `example_headers` (in the
    dependency order registry.yaml declares them) with `example_code`,
    stripping inter-bridge includes/`#pragma once` from all of them,
    into one standalone translation unit -- no `-I` include path
    needed to compile it, since every bridge header it depends on is
    now inlined directly."""
    parts = [strip_internal_lines((REPO_ROOT / h).read_text()) for h in facility.get("example_headers", [])]
    parts.append(strip_internal_lines(example_code))
    return "\n\n".join(parts) + "\n"


def compile_check_example(facility, flattened_source, page_name):
    """Compiles and runs `flattened_source` standalone under this
    project's floor standard (C++17), the same generic `c++` (not a
    hardcoded g++/clang++) invocation `./bridge probe` uses, so it
    picks up whichever devShell's compiler is on `PATH`. Raises with
    the compiler's/binary's own output on either failure -- a broken
    on-page Example fails `./bridge docs` loudly, the same
    empirical-verification discipline this project applies everywhere
    else, guarding the docs examples against silent rot as headers
    change."""
    with tempfile.TemporaryDirectory() as tmp:
        src_path = Path(tmp) / "example.cpp"
        bin_path = Path(tmp) / "example_bin"
        src_path.write_text(flattened_source)
        compile_result = subprocess.run(
            ["c++", "-std=c++17", "-Wall", "-Wextra", "-Werror", "-o", str(bin_path), str(src_path)],
            capture_output=True,
            text=True,
        )
        if compile_result.returncode != 0:
            raise RuntimeError(
                f"{page_name}: flattened Example failed to compile under -std=c++17:\n{compile_result.stderr}"
            )
        run_result = subprocess.run([str(bin_path)], capture_output=True, text=True)
        if run_result.returncode != 0:
            raise RuntimeError(
                f"{page_name}: flattened Example compiled but failed at runtime "
                f"(exit {run_result.returncode}):\n{run_result.stdout}{run_result.stderr}"
            )


def verify_example(facility, content):
    """Extracts, flattens, and compile-checks `facility`'s Example
    section against `content` (the page's *current*, not yet
    regenerated, hand-written prose -- the Example section itself is
    never machine-owned). No-op if the page has no Example section
    yet (a freshly-scaffolded facility)."""
    example_code = extract_example_code(content, facility["page"])
    if example_code is None:
        return
    flattened = flatten_example(facility, example_code)
    compile_check_example(facility, flattened, facility["page"])


BLOCK_RENDERERS = {
    "header-link": render_header_link_block,
    "headers": render_headers_block,
    "symbols": render_symbols_block,
}


def scaffold_content(facility):
    page_id = "page_" + facility["name"].replace("-", "_")
    title = facility["name"].replace("-", " ").title()
    parts = [f"\\page {page_id} {title}\n"]
    for tag in ("header-link", "headers", "symbols"):
        parts.append(f"{BLOCK_BEGIN.format(tag=tag)}\n{BLOCK_END}\n")
    parts.insert(3, "TODO: narrative prose for this facility.\n")
    return "\n".join(parts)


def replace_block(content, tag, rendered_text, page_name_for_errors):
    """Rewrites the single BRIDGE-DOCS marker block for `tag` in
    `content`. Content outside the markers is untouched."""
    begin = BLOCK_BEGIN.format(tag=tag)
    pattern = re.compile(re.escape(begin) + r".*?" + re.escape(BLOCK_END), re.DOTALL)
    replacement = f"{begin}\n{rendered_text}{BLOCK_END}"
    if pattern.search(content) is None:
        raise ValueError(f"missing marker block {tag!r} in {page_name_for_errors}")
    return pattern.sub(lambda _m, r=replacement: r, content, count=1)


def regenerate_blocks(content, facility):
    """Rewrites every BRIDGE-DOCS marker block in `content` using this
    facility's current data."""
    for tag, renderer in BLOCK_RENDERERS.items():
        content = replace_block(content, tag, renderer(facility), facility["page"])
    return content


def render_facility_index_block(facilities):
    # \ref, not \subpage: several of these (detectors/feature-tests/
    # diagnostics) are already \subpage-nested under rivets.md's
    # hand-written overview, and Doxygen's page tree only allows one
    # \subpage parent per page. This block is a supplementary flat
    # A-Z-style index, not the primary hierarchy -- the hand-written
    # prose around it establishes that via \subpage instead.
    lines = []
    for fac in sorted(facilities, key=lambda f: f["name"]):
        title = fac["name"].replace("-", " ").title()
        page_id = "page_" + fac["name"].replace("-", "_")
        lines.append(f"- \\ref {page_id} \"{title}\"")
    return "\n".join(lines) + "\n"


def regenerate_index_block(content, facilities):
    return replace_block(content, "facilities", render_facility_index_block(facilities), "index.md")


def process(facilities, write):
    """Returns a list of (page_path, old_content_or_None, new_content)
    for every facility, writing to disk if `write` is True."""
    results = []
    for facility in facilities:
        page_path = PAGES_DIR / facility["page"]
        if page_path.exists():
            old_content = page_path.read_text()
            verify_example(facility, old_content)
            new_content = regenerate_blocks(old_content, facility)
        else:
            old_content = None
            new_content = scaffold_content(facility)
            new_content = regenerate_blocks(new_content, facility)
        results.append((page_path, old_content, new_content))
        if write:
            page_path.write_text(new_content)

    # docs/pages/index.md isn't a facility itself (docs/adr/0014) --
    # it's the hand-written mainpage -- but it carries one generated
    # block of its own: a bullet list `\subpage`-linking every
    # registered facility, kept in sync the same way.
    index_path = PAGES_DIR / "index.md"
    if index_path.exists():
        old_index = index_path.read_text()
        new_index = regenerate_index_block(old_index, facilities)
        results.append((index_path, old_index, new_index))
        if write:
            index_path.write_text(new_index)

    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("mode", choices=["generate", "check"])
    args = parser.parse_args()

    facilities = load_registry()

    missing = check_coverage(facilities)
    if missing:
        print("error: headers not assigned to any facility in docs/pages/registry.yaml:", file=sys.stderr)
        for h in missing:
            print(f"  {h}", file=sys.stderr)
        sys.exit(1)

    try:
        if args.mode == "generate":
            process(facilities, write=True)
            print(f"Regenerated {len(facilities)} facility page(s).")
            return

        # check mode: compute what generate *would* write, diff against
        # what's actually committed, fail on any mismatch.
        results = process(facilities, write=False)
    except (RuntimeError, ValueError) as exc:
        print(f"error: {exc}", file=sys.stderr)
        sys.exit(1)
    failed = False
    for page_path, old_content, new_content in results:
        if old_content is None:
            print(f"error: {page_path} does not exist -- run `scripts/docs-pages.py generate` first", file=sys.stderr)
            failed = True
            continue
        if old_content != new_content:
            rel = page_path.relative_to(REPO_ROOT)
            diff = difflib.unified_diff(
                old_content.splitlines(keepends=True),
                new_content.splitlines(keepends=True),
                fromfile=f"{rel} (committed)",
                tofile=f"{rel} (regenerated)",
            )
            print(f"error: {rel} is out of date with the registry/Doxygen XML:", file=sys.stderr)
            sys.stderr.writelines(diff)
            failed = True
    if failed:
        sys.exit(1)
    print(f"{len(results)} facility page(s) up to date.")


if __name__ == "__main__":
    main()
