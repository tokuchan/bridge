# A Facility-Based Documentation Site, Generated from a Registry

## Context

Doxygen's coverage gate (`WARN_IF_UNDOCUMENTED`, `WARN_AS_ERROR`)
guarantees every symbol has a doc comment, but says nothing about
structure above that. Before this ADR, the only hand-written page was
README.md as the mainpage; there was no page describing a conceptual
division that spans multiple headers as one thing. This is a real gap:
`format`/`print` alone spans four header files across `truss/cpp17/`
and `deck/cpp17/`, and nothing tied them together into a single "here
is the format/print facility" page the way cppreference.com ties
`std::format`'s whole family together under one entry.

Separately, bridge had no GitHub Actions workflows and no hosted,
browsable copy of its documentation at all -- only whatever a
contributor built locally via `./bridge docs`.

## Decision

### A hand-maintained facility registry is the unit of division

`docs/pages/registry.yaml` declares a set of **facilities** -- a
conceptual division a user would actually look up (`optional`,
`expected`, `format-print`, `detectors`, `feature-tests`,
`diagnostics`, `version`) -- each owning a list of header files, a
`docs/pages/<name>.md` page, and (nullable) the one cppreference.com
URL that facility mirrors. Every header under `include/**/*.hpp` must
be assigned to exactly one facility; `scripts/docs-pages.py` fails
`./bridge docs` outright otherwise, the same severity Doxygen's own
`WARN_IF_UNDOCUMENTED` already gives an undocumented symbol. This was
chosen over two simpler alternatives considered and rejected:

- **One page per real `std::` header.** Doesn't fit Rivets, which
  mirrors no single `std::` header at all -- it's a bridge-only
  detection concept.
- **One page per top-level library** (Rivets/Truss/Deck, three pages
  total). Loses exactly the granularity motivating this ADR --
  `format`/`print` would end up buried inside one giant Truss page
  alongside `optional` and `expected`.

Rivets itself splits into three facilities (`detectors`,
`feature-tests`, `diagnostics`), matching CONTEXT.md's own conceptual
boundaries, with a hand-written overview page (`docs/pages/rivets.md`)
above them discussing the shared "sparse Cartesian product" mental
model and the `constexpr`-vs-`#if` dual-form rationale that spans all
three. That overview page is deliberately *not* itself a registry
facility -- it owns no headers of its own, and this establishes the
general pattern for any future library-level overview page: hand-written,
`\subpage`-linking down into registry-tracked facility pages, not
tracked by the registry itself.

### Pages are Doxygen `\page` files, generated from the registry

Narrative content lives in `docs/pages/<facility>.md` (Doxygen `\page`
directive, `docs/pages/` added to `INPUT`), rendered alongside the
auto-generated class/namespace/file docs so `@ref`/`\subpage` can link
straight into extracted symbol documentation. Considered and rejected:
a separate static-site generator (two toolchains to keep in sync,
fragile cross-linking into Doxygen's own generated pages) and
hand-written-only pages with no sync mechanism at all (drifts silently
as the registry or the code changes).

Each facility page has two machine-owned regions, delimited by HTML
comment markers (`<!-- BRIDGE-DOCS:BEGIN headers -->`...`<!--
BRIDGE-DOCS:END -->`, and the same for `symbols`), that
`scripts/docs-pages.py` rewrites on every docs build; hand-written
prose outside the markers is never touched. `docs/pages/index.md` (the
mainpage) carries one such block too -- a flat, auto-updated list of
every registered facility. This regenerate-in-place model was chosen
over generate-once-and-never-touch-again: the header/symbol lists
would otherwise silently drift out of sync with the registry and the
code over time.

The `symbols` block is a cppreference-style table (name, kind, brief),
sourced from Doxygen's own XML output (`GENERATE_XML = YES`) rather
than hand-duplicated in the registry -- the brief text is the same text
`WARN_IF_UNDOCUMENTED` already enforces exists, so the table can't
drift from the real docs the way a hand-maintained copy could. Finding
the *right* symbols to include (not every documented internal helper
Doxygen also extracts from the same detail namespace) means parsing
each header for its actual `using bridge::detail::...::name;` promotion
lines (docs/adr/0001) and filtering to just those names; a header with
no such lines (Rivets' detection headers, which have no detail/exports
tiering at all) is treated as fully public instead, since there's
nothing to filter against.

Two further, non-obvious wrinkles the implementation surfaced (not
anticipated during design, found by actually building the site):

- **A local name is not always a stable identity.** Truss defines the
  real symbol; Deck's own alias-selection header (its two-part answer
  to which is a passthrough alias and which is the polyfill) often
  re-declares the *same* public name as a thin, separately-documented
  alias. The two must collapse to one row (preferring the substantive
  definition), which is a *good* thing to collapse -- but Rivets'
  Detectors deliberately reuse the *same* local name (`eq`, `ge`, ...)
  across genuinely distinct per-Entity namespaces
  (`bridge::rivets::gcc::eq` is not `bridge::rivets::clang::eq`), which
  must *not* collapse. The distinguishing rule: a symbol whose
  enclosing namespace matches the Truss/Deck detail-tiering pattern
  collapses on its bare name; everything else keeps its qualifying
  namespace as part of its display identity.
- **A `\subpage` cycle is a hard Doxygen crash, not a warning.** Two
  pages `\subpage`-ing each other (the Rivets overview subpaging its
  three facilities, one of which linked back via `\subpage` instead of
  `\ref`) segfaulted the Doxygen binary outright rather than producing
  a diagnosable error. `\ref` for any same-tree backreference; `\subpage`
  reserved strictly for the one-parent-per-page tree edge it's meant
  to establish.

### Two-pass Doxygen build

The `symbols` block's content depends on *this run's own* XML output,
but that same run also needs to render the just-regenerated page
content into HTML. `./bridge docs` therefore runs Doxygen once (XML +
HTML from whatever the pages currently say), regenerates the marked
blocks from that XML via `scripts/docs-pages.py generate`, then runs
Doxygen a second time so the final HTML reflects the now-current
pages. Slower than a single pass, but correctness -- a CI `check` run
must compare against genuinely current data -- outweighs the extra
build time for a project this size. Confirmed the regenerated output
is identical across two separate compiler devShells (default GCC, then
`clang_20`) before relying on this determinism.

### Two enforcement gates, matching the existing coverage-gate severity

- **Unassigned header = hard failure.** Any header not listed under
  some facility's `headers` fails `./bridge docs`, not just a warning.
- **Stale generated block = CI failure, not a local one.** A separate
  `scripts/docs-pages.py check` mode regenerates into a temporary copy
  and diffs it against the committed page, failing with the diff shown
  on any mismatch. This runs in CI, not local `./bridge docs` (which
  regenerates in place so a contributor always sees current output
  locally) -- it exists specifically to catch a stale or
  hand-edited-inside-the-markers page landing on `master`.

### cppreference cross-linking, both per-symbol and per-page, retrofitted

Every promoted symbol's own doc comment gets an `@see` line to its
facility's cppreference.com URL (one URL per facility, pulled from the
same registry entry the table uses -- not duplicated by hand per
symbol), and the facility page's generated table repeats that same
link on every row. Applied retroactively to `optional`, `expected`,
and `format`/`print` -- the three facilities that existed before this
ADR -- matching the precedent ADR-0011 set for its divergence-note
policy: fix the gap between old and new facilities rather than leaving
a permanent, silently-inconsistent split.

`format-print` is the one facility with a real simplification here:
`print`/`println` have their own separate cppreference page
(`cpp/io/print`), but the registry only carries one URL per facility,
pointed at `format`'s own page instead. Accepted as a disclosed
simplification (the facility's own page prose links both), not fixed
by making `cppreference` a list -- one link per facility is enough for
the "what does the standard say" pointer this exists to give, and a
list reopens exactly the "which row gets which link" complexity the
single-URL-per-facility design was chosen to avoid.

### GitHub Pages via a docs-only, Nix-based GitHub Actions workflow

`github.com/tokuchan/bridge` is public, so GitHub Actions (unlimited
minutes) and Pages hosting are both free -- confirmed before deciding
this, not assumed, since cost was a real, explicitly-raised concern.
One workflow, `.github/workflows/docs.yml`: on push to `master`,
install Nix, then run the same underlying commands
`commands/docs/main.sh` runs, inside `nix develop` against the
project's own `flake.nix` default devShell -- not through the
container-wrapping `./bridge` script itself, whose isolation purpose is
redundant on an already-ephemeral GitHub-hosted runner. This extends
ADR-0003's Nix-provided-tooling premise into CI for the first time:
zero drift between what a contributor's `nix develop` gives them
locally and what actually builds the deployed site. Deliberately
**not** expanding into a full test-running CI in this same effort --
`./bridge test`/the compiler matrix stay exactly as manual/local as
they've always been; wiring those into CI is a separate decision for
another day, not a natural extension of "host the docs."

### CONTEXT.md and every ADR fold into the site as plain Markdown, unmodified

Both were originally left outside Doxygen's `INPUT` entirely, so every
facility page's "see CONTEXT.md"/"see ADR-0010" reference sent a reader
off-site to GitHub for the actual content. Both are now added to
`INPUT` (`Doxyfile.in`) as-is -- no `\page` directive injected into
either file. Doxygen renders a `.md` file into a page even without one,
auto-titling and auto-generating its page ID from the file path;
cross-linking (`\ref`, or a plain relative Markdown link the way
CONTEXT.md already links to ADRs) still resolves correctly against
that auto-generated name, confirmed by building the site and checking
the rendered links, not assumed.

Considered and rejected: injecting `\page adr_0010 ...` at the top of
each ADR for cleaner page IDs. Both files are explicitly meant to be
read directly on GitHub too (an ADR's whole purpose includes being
skimmable in a PR diff or a repo browse) -- Doxygen-specific markup
would render as stray literal text there, a real cost for a purely
cosmetic gain in page-ID friendliness.

## Consequences

- `python3` (with `pyyaml` via `python3.withPackages`, not `uv`) joins
  the default devShell's packages. `uv` was considered and rejected:
  it would introduce a second, non-Nix, PyPI-facing package-resolution
  mechanism alongside this project's existing Nix-hermetic model, and
  the tooling needs nothing beyond stdlib plus one real dependency
  (PyYAML, for a hand-edited registry file where comments and clean
  multi-line lists matter). If a further third-party dependency
  becomes necessary later, the same `withPackages` mechanism handles
  it without introducing a new package manager.
- The Doxygen Awesome theme (`third_party/doxygen-awesome-css`, a git
  submodule pinned to `v2.4.2`, matching the existing `.run` submodule
  precedent) replaces the default Doxygen HTML look, for a cleaner,
  sidebar-navigated site closer to cppreference's own aesthetic.
  `HTML_COLORSTYLE = LIGHT` disables Doxygen's own native dark-mode
  toggle so the theme's own toggle is the only one, not two competing
  buttons.
- `README.md` is no longer Doxygen's mainpage source
  (`USE_MDFILE_AS_MAINPAGE` removed); it stays the lightweight
  GitHub-facing quick-start, and `docs/pages/index.md` (via its own
  `\mainpage` command) becomes the hand-curated hierarchical index
  instead. The two audiences -- a GitHub visitor skimming the repo, and
  someone browsing the built site -- were being conflated by one file
  serving both.
- Every future facility (a new Truss/Deck polyfill, a new Rivets
  Entity) needs a registry entry and a page alongside its ADR and
  CONTEXT.md updates -- this joins the existing "read every ADR before
  committing" `CLAUDE.md` check as something to verify, not just
  something to remember to write.
- The `format-print` cppreference simplification (one URL, not
  per-symbol) means a reader following the `print`/`println` rows'
  cppreference link lands on `format`'s cppreference page, not
  `print`'s own -- disclosed here and in the page's own prose, not
  silently wrong.
- Facility pages can now `\ref`/link to a specific ADR or to
  CONTEXT.md's glossary entries in-site, instead of only ever pointing
  out to a GitHub blob URL -- existing facility pages still use the
  GitHub-URL form from when this ADR first shipped; migrating them to
  in-site links is incremental cleanup, not required by this change.
