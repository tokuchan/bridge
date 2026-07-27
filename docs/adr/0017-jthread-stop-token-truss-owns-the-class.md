# For `jthread`/`stop_token`, Truss Owns the Classes; `stop_callback` Deferred

## Context

`std::jthread`/`std::stop_token`/`std::stop_source` don't exist at all
before C++20 — like `expected` ([ADR-0010](0010-expected-truss-owns-the-class.md))
and `span` ([ADR-0015](0015-span-truss-owns-the-class.md)), there's no
pre-existing C++17 type for Truss to attach free functions onto. Unlike
either of those, this facility involves real concurrency: `jthread`
wraps a joinable OS thread, and `stop_token`/`stop_source` coordinate
cooperative cancellation across threads via shared state.

Two things confirmed by direct probe before any code was written, not
assumed:

- **A real libstdc++ gap.** `__cpp_lib_jthread` is defined (`201911L`)
  and the feature is fully functional — confirmed by actually
  compiling and running a `std::jthread` constructed with a
  `stop_token`-accepting lambda — on every GCC in this project's
  matrix (13, 14, 15) and on Clang 20 (same libstdc++ underneath on
  this host). `__cpp_lib_stop_token` is **never** defined, on any of
  them, even with `<stop_token>` included directly. `stop_token`
  itself works; libstdc++ just doesn't publish the macro the standard
  says it should.
- **No platform-specific code is needed.** The whole facility is
  expressible over `std::thread` + `<atomic>`/`<mutex>`/
  `<condition_variable>` — all already-portable C++17 facilities. The
  RAII-join/request-stop-on-destruction behavior `jthread` adds on top
  of `std::thread` needs no OS-specific primitives.

## Decision

### Truss owns full, from-scratch classes

`bridge::truss::stop_token`, `bridge::truss::stop_source`, and
`bridge::truss::jthread` are complete classes, matching `expected`'s
and `span`'s precedent: no pre-existing STL type to extend, so Truss
owns the whole shape. Truss's classes never themselves pass through to
the real `std::` equivalents, even under C++20/23 — that selection is
Deck's job alone, same invariant as every other Truss-owned facility.

### `stop_callback` is deferred, not implemented this pass

`stop_callback<Callback>`'s destructor has a real concurrent-
correctness contract, not just missing convenience: it must **block**
if its callback is currently executing on another thread, but must
**not** deadlock if it's being destroyed from inside its own callback
(self-referential-destruction detection). Implementing this correctly
needs its own carefully-tested concurrent state machine — genuinely
the highest-risk, hardest-to-verify code this facility would need, and
a real undefined-behavior risk if rushed, not merely an incompleteness.

Deferred as a disclosed, follow-up-worthy gap, matching this project's
existing precedent for scope trims (`format`/`print`'s range-formatting
trim, `span`'s tuple-like-interface trim, [ADR-0012](0012-format-print-truss-owns-the-facility.md)/
[ADR-0015](0015-span-truss-owns-the-class.md)). `stop_token`/
`stop_source`/`jthread` alone already cover cooperative cancellation's
actual common case (a `jthread` polling `stop_requested()`);
callback-based cancellation is rarer. When `stop_callback` is
eventually attempted, its own Feature Test threshold needs the same
empirical confirmation this ADR gave `jthread`/`stop_token` — not
assumed from the standard's documented value.

### Feature Test gate: a Detector-backed override, not a bare check

Deck's passthrough condition for the whole facility:

```
BRIDGE_RIVETS_FEATURES_LIB_JTHREAD >= 201911L
&& (
     BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN >= 201907L
     || bridge::rivets::libstdcxx::version > 0   // the override
   )
```

`jthread`'s own Feature Test is always required — the baseline "does
this stdlib have the facility at all" signal, and the one actually
confirmed live on every toolchain in this project's matrix.
`stop_token`'s own Feature Test is checked normally for any *other*
standard library (libc++, MSVC STL); the override is scoped
specifically to "libstdc++ is confirmed active" via
`rivets/libstdcxx.hpp`'s Detector (implemented ahead of its own place
in the Rivets roadmap specifically for this), not applied blindly
everywhere — so a hypothetical future stdlib bug in the *opposite*
direction (a macro present without real support) isn't silently
papered over on some other stdlib.

`201907L` is the standard's documented value for
`__cpp_lib_stop_token` — **not** empirically confirmed against any
real toolchain in this project's matrix, since none of them ever
define it. Disclosed as such rather than presented as verified; worth
re-checking the moment any toolchain in the matrix actually publishes
the macro.

This is a genuinely different shape than every prior Feature-Test-only
gate this project has used ([ADR-0007](0007-feature-test-wrapping.md),
[ADR-0008](0008-best-effort-head-standard.md)): a Detector, normally
reserved for "no SD-6 macro exists for this at all," here corrects a
*specific, confirmed-wrong* Feature Test signal rather than substitute
for a missing one.

### One facility, "jthread"

`stop_token`/`stop_source`/`jthread` are documented and registered as
one facility (`docs/pages/jthread.md`, one `docs/pages/registry.yaml`
entry) — mirroring `format-print`'s precedent of bundling tightly
coupled symbols rather than one page per type. `jthread`'s own real
header (`<thread>`) is used for the registry's `header:`/prominent-
link/table-stub purposes, the more prominent/user-facing name;
`stop_token`/`stop_source`'s real header (`<stop_token>`) is disclosed
in the page's own prose as a simplification, the same shape
`format-print` already uses for `print`/`println` living in `<print>`
not `<format>`.

### `Threads::Threads` links unconditionally

This is the first facility needing a real system library rather than
pure header-only compilation. `truss` (and transitively `deck`) links
`Threads::Threads` via CMake's `FindThreads`, unconditionally — not
gated behind a new opt-in flag the way Boost is
([ADR-0002](0002-boost-optional-feature-detected.md)). A system
threading library is essentially universal on any target this project
supports, unlike Boost (a genuinely heavy, optional third-party
dependency); gating it would be needless ceremony for something
realistically always available. Confirmed the link still resolves
cleanly on this project's own toolchain, where modern glibc folds
pthread into libc itself — `Threads::Threads` correctly adds no extra
flag there, rather than this project assuming `-pthread`/`-lpthread`
is always needed.

## Consequences

- `truss`/`deck` are no longer purely header-only in the sense of
  "needs nothing beyond the standard library" — every consumer now
  links a real system threading library, whether or not they touch
  `jthread`. Accepted given how close to universal that dependency
  actually is.
- A consumer relying on `stop_callback` has no polyfill to use before
  their ecosystem's own `std::stop_callback` arrives — a real,
  disclosed gap in this pass's coverage, not a silent one.
- `rivets/libstdcxx.hpp` exists and is implemented earlier than its
  own natural place in the Rivets roadmap would have put it, driven
  entirely by this facility's own needs — a precedent for pulling a
  Detector forward when a real Feature Test gap demands it, rather
  than always following registry order strictly.
- The Detector-override pattern established here (a Feature Test's
  documented value contradicted by a specific, confirmed stdlib
  behavior) is a template for any future facility that hits the same
  shape of surprise — check the Feature Test's *actual* behavior per
  stdlib before trusting it blindly, and reach for a Detector override
  when it's specifically wrong, not just absent.
