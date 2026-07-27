# Compiler Explorer Links: Attempted, Rejected -- Compile-Check Gate Ships Instead

## Context

Every facility page now has a hand-written Example (docs/adr/0014's
amendment). A reader can already copy that snippet into their own
editor, but there's no one-click way to actually run it against a real
compiler -- the kind of "try it" link cppreference and most modern
library docs sites offer.

Compiler Explorer (godbolt.org) is the natural target: it's already
the tool this project's own contributors reach for, and it supports
this project's exact compiler matrix (GCC/Clang, multiple versions).
Three integration paths were considered:

- **Official "add as a library"** (the Libraries dropdown): requires a
  public GitHub repo with tagged releases, plus an upstream PR to
  `compiler-explorer/infra` and `compiler-explorer/compiler-explorer`
  (or their CE Library Wizard CLI automating both). Needs the
  maintainers' review and a real tagged `v`-prefixed release to point
  at. Not attempted here -- external approval and a real release are
  both prerequisites this pass doesn't have.
- **Raw `#include`-by-URL**, a real, already-shipped Compiler Explorer
  feature: does **not** resolve transitive local includes (confirmed
  against a real compiler-explorer/compiler-explorer issue, #3929).
  Bridge's headers are exactly this shape (Deck includes Truss includes
  Rivets), so this doesn't work as-is. Not attempted.
- **ClientState-encoded permalinks**: Compiler Explorer's
  `/clientstate/<base64>` endpoint (documented in the project's own
  `docs/API.md`) encodes an entire session -- source, compiler,
  options -- directly in a URL, no server-side storage needed.
  Buildable with no upstream approval required -- the only path
  actually attempted, and the subject of this ADR.

## Decision

**Rejected.** Built the ClientState mechanism, generated a real link
against the `optional` facility's flattened example, and had it
manually click-tested (this project has no browser-automation tooling,
so a generated permalink's actual behavior on Compiler Explorer's own
site can only be confirmed by a human opening it). Compiler Explorer
refused to parse the link. The generated code (flattening + JSON
encoding + link generation) is not merged; only this ADR and the
still-valuable half described below are.

### Why it failed

Two compounding problems, either alone possibly survivable, together
fatal for this iteration:

- **The flattened source is too large.** A facility's Example can't be
  handed to Compiler Explorer as source text alone -- it `#include`s
  bridge's own headers by path, which only resolve on this project's
  filesystem. The fix (concatenate the facility's real headers,
  `#include`s/`#pragma once` stripped, ahead of the example -- see
  "What ships" below) works and compiles, but produces a *complete,
  comments-and-all* translation unit: 29KB for the smallest facility
  (`optional`), over 100KB for the largest (`expected`,
  `format-print`). Base64 inflates that by another third
  (`~40KB`-`~143KB` per generated URL). Compiler Explorer's
  `/clientstate/<base64>` endpoint rejected a URL this size outright,
  confirmed by the actual click-test above, not assumed from a size
  limit written down anywhere.
- **The encoding recipe itself is unconfirmed.** Compiler Explorer's
  own maintainers have an open, unresolved issue (#3075) acknowledging
  that the exact `/clientstate/<base64>` recipe -- which base64
  alphabet, whether the result needs further percent-encoding once
  placed in a URL path -- isn't actually documented anywhere
  authoritative. The size failure means this second uncertainty was
  never actually resolved either way; a correctly-sized payload might
  still need a different encoding than the one tried.

### Why not just shrink it further

Two paths could reduce the payload, both rejected for this pass:

- **Strip comments from the flattened source before encoding.** Cuts
  size substantially (these headers are comment-heavy by this
  project's own documentation standards), but real block comments
  (`/** ... */`) appear multi-line, inside X-macro definitions with
  `\`-continued lines
  (`include/deck/cpp17/optional.hpp`'s `BRIDGE_DECK_OPTIONAL_DEFINE_COMPARISON`,
  `include/rivets/detail/detector.hpp`'s Named Detector macros). A
  naive line-based stripper (matching the `strip_internal_lines()`
  approach that already handles `#include`/`#pragma once` safely) is
  not obviously safe against these without real block-comment-aware
  state tracking -- a meaningfully riskier piece of code than anything
  else this pass touched, for a feature already independently blocked
  by the unresolved encoding question above. Rejected as
  disproportionate risk for uncertain payoff.
- **zlib/deflate-compress the payload before base64** (the endpoint
  documents auto-detecting this): shrinks the encoded size roughly
  5x in a spot check (`expected`: 137KB plain-base64 down to 23KB
  deflated), but reintroduces exactly the determinism fragility
  avoided by choosing plain base64 in the first place -- deflate
  output isn't guaranteed byte-identical across zlib versions, so a
  future nixpkgs bump (or a contributor running `scripts/docs-pages.py
  check` outside the pinned Nix devShell) could silently change the
  generated URL and produce a confusing CI diff. Rejected for the same
  reason plain base64 was chosen originally: this generator's output
  needs to stay deterministic for `check` mode, and correctness (does
  the link work at all) hadn't even been established yet when this
  tradeoff came up -- not worth taking on a determinism risk to shrink
  a URL that was already broken for other reasons.

Both remain legitimate future options if interactive links are
revisited -- comment-stripping in particular directly addresses a
second problem compression alone wouldn't: even a successfully-loaded
link would drop a reader onto hundreds of lines of bridge internals
with the actual short example buried at the bottom, not the compact,
readable snippet the page itself shows.

## What ships instead

The flattening transform and a compile-verification gate, independent
of whether an interactive link exists on top:

`docs/pages/registry.yaml` gains an `example_headers` field per
facility -- the dependency-ordered (rivets -> truss -> deck) list of
real bridge headers the Example needs, hand-declared rather than
auto-walked from `#include` directives (the facility's own author
already knows the real order; an auto-walker is more moving parts for
equivalent value here). `scripts/docs-pages.py` concatenates those
files' contents with the Example's own code into one translation unit,
stripping every inter-bridge `#include <rivets|truss|deck/...>` line
and every `#pragma once` line from all of them first (confirmed
necessary by hand-flattening `optional` first and hitting real
redefinition errors without this: `#pragma once` doesn't dedupe a
pasted-in copy against the same file also reachable by its real
`#include` path, since the paste is now part of the same translation
unit, not a header being `#include`d). The result needs no `-I`
include path at all -- every bridge header it depends on is already
inlined.

That flattened form is compiled and run under `-std=c++17 -Wall
-Wextra -Werror`, via the same generic `c++` invocation `./bridge
probe` uses (whichever compiler the active devShell put on `PATH`), as
part of both `scripts/docs-pages.py generate` and `check`. A broken
Example -- one that no longer compiles, or whose assertions no longer
hold, once the real headers it flattens drift out from under it --
fails `./bridge docs` loudly, with the compiler's or the binary's own
output. Verified by deliberately breaking one Example and confirming
the gate catches it before relying on this. C++17 (this project's
floor) is the one standard checked, regardless of facility -- even
Rivets' own facilities (no polyfill/passthrough split) compile fine
under it, and checking uniformly is simpler than tracking a
per-facility floor that's the same value everywhere in this codebase
today.

## Consequences

- `scripts/docs-pages.py check` (CI) needs a real C++ compiler on
  `PATH`, not just Python and Doxygen -- a new dependency for the
  GitHub Actions workflow docs/adr/0014 set up, deliberate and
  disclosed here rather than discovered as a surprise CI failure.
- A facility's `example_headers` list is hand-maintained, separate
  from `headers` (which Doxygen's XML scan uses) -- adding a new file
  to a facility's real header list doesn't automatically add it to its
  example's flattened form; an author has to update both when an
  Example starts depending on something new.
- No facility page has a "Try it" link. A reader wanting to actually
  run an Example still has to copy it into their own editor -- the gap
  this ADR set out to close remains open, disclosed here rather than
  silently absent.
- The path back, if this is revisited: resolve the encoding-recipe
  uncertainty (#3075) and the size problem together, most likely via a
  real block-comment-aware minifier for the flattened source (not
  compression, per the determinism concern above) tested against a
  size-representative facility (`expected` or `format-print`, not
  `optional`) before generating links for all eight.
