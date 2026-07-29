# For `source_location`, Truss Owns a Full Class, with Two Disclosed Fidelity Gaps

## Context

`std::source_location` doesn't exist at all before C++20 — like
`expected` ([ADR-0010](0010-expected-truss-owns-the-class.md)), `span`
([ADR-0015](0015-span-truss-owns-the-class.md)), and `jthread`/
`stop_token` ([ADR-0017](0017-jthread-stop-token-truss-owns-the-class.md)),
there's no pre-existing STL type from an older standard for Truss to
attach free functions onto. Unlike any of those, `source_location`'s
entire value comes from a `consteval`/`constexpr` factory function
(`current()`) that captures its caller's call site through compiler
builtins, evaluated as a default argument. C++17 has no `consteval`,
so the polyfill uses plain `constexpr` instead — the standard's own
well-known pre-C++20 workaround.

Two things confirmed by direct probe before this ADR was written, not
assumed:

- **A trustworthy Feature Test.** `__cpp_lib_source_location` reports
  `undef` under `-std=c++17` and exactly `201907L` (the P1208R6 paper
  value) under `-std=c++20`, on this project's own GCC. Unlike
  `jthread`/`stop_token` ([ADR-0017](0017-jthread-stop-token-truss-owns-the-class.md)),
  this signal needs no Detector-backed override — Deck's passthrough
  choice is a bare Feature Test check.
- **Two real fidelity gaps in the polyfill**, both surfaced by
  building and testing the polyfill against the real facility, not
  anticipated up front. See "Fidelity scope" below.

## Decision

Truss owns a complete, from-scratch `source_location` class (following
`expected`/`span`/`stop_token`'s precedent: a full class when there's
no existing type to extend), with a static `current()` factory and
`file_name()`/`function_name()`/`line()`/`column()` accessors.
`bridge::truss::source_location` is **unconditionally** Truss's own
class, regardless of standard or toolchain — Truss never passes
through, even under C++20 where the real type is available.
Passthrough selection happens exactly once, in Deck:
`bridge::deck::source_location` (promoted to `bridge::source_location`)
is a plain type alias to `std::source_location` when
`BRIDGE_RIVETS_FEATURES_LIB_SOURCE_LOCATION` confirms it, or to
`bridge::truss::source_location` otherwise — the same shape `span`'s
and `stop_token`'s Deck headers use, since Truss's class already has
the target shape and there's nothing left for Deck to wrap.

### Fidelity scope

Per [ADR-0008](0008-best-effort-head-standard.md)'s "behaviorally
indistinguishable" bar, two gaps are disclosed rather than matched,
each for a different reason:

- **`column()`: a compiler split.** GCC has never implemented a public
  `__builtin_COLUMN()`, on any version, confirmed by direct probe
  against every GCC in this project's matrix (13, 14, 15) — real
  `std::source_location` still reports a correct column on GCC, but
  only through libstdc++'s own private, non-portable
  `__builtin_source_location()`, gated internally on
  `__has_builtin(__builtin_source_location)` in libstdc++'s own
  headers. A third-party polyfill can't rely on that private builtin.
  Clang has had a public `__builtin_COLUMN()` since Clang 9, well
  below this project's matrix floor (Clang 18), confirmed by direct
  probe. The polyfill's `column()` uses
  `#if BRIDGE_RIVETS_CLANG_GE(9)` to select the real builtin on Clang,
  and falls back to a fixed `0` everywhere else — this project's own
  `rivets/clang.hpp` already exposes the needed comparator as a plain,
  arbitrary-`n` `#if`-usable macro (`BRIDGE_RIVETS_CLANG_GE(n)`), so no
  new Detector file or Named Detector was needed for this check.
- **`function_name()`: a standard-library-wide fact, not a compiler
  split.** Discovered only once the polyfill was built and tested
  directly against real `std::source_location` — not something the
  original design anticipated. Real
  `std::source_location::function_name()` returns a full function
  signature on libstdc++ (for example `"int main()"`), confirmed by
  direct probe. The only portable compiler builtin available to a
  polyfill, `__builtin_FUNCTION()`, returns just the bare name (for
  example `"main"`). Confirmed identical on GCC and on Clang paired
  with libstdc++ — this project's whole compiler matrix uses
  libstdc++, no pairing in the matrix tests libc++ — so, unlike
  `column()`, this gap is not conditioned on `BRIDGE_RIVETS_CLANG_GE`;
  it applies to every toolchain this project's matrix covers alike.
  Matching the real signature format would need a
  `__PRETTY_FUNCTION__`-style extension instead, whose own formatting
  isn't identical across GCC and Clang, isn't guaranteed to match
  libstdc++'s internal formatting exactly, and is a materially bigger
  change to the polyfill's design for uncertain fidelity gain. Decided
  against, in favor of disclosing the gap plainly.

Both gaps get a `BRIDGE_RIVETS_DIVERGENCE_NOTE`
([ADR-0011](0011-warn-on-surprising-facility-divergences.md)) in
`deck/cpp17/source_location.hpp`'s polyfill branch: the
`function_name()` note fires unconditionally there, the `column()`
note fires only when `!BRIDGE_RIVETS_CLANG_GE(9)` — matching each
gap's own actual scope, not a single blanket note for both.

## Consequences

- A consumer relying on `column()`'s exact value, or on
  `function_name()`'s exact format, sees different behavior depending
  on which toolchain is active and whether the same build later
  upgrades to real C++20 passthrough — both are disclosed, compiler-
  visible facts at the point the polyfill branch compiles, not silent
  surprises.
- `line()` and `file_name()` have no equivalent gap: both are matched
  exactly against real `std::source_location`, confirmed by a
  differential test comparing the polyfill and the real facility
  captured at the same call site.
- This ADR's `function_name()` finding was not part of this facility's
  original design grill — it surfaced only once the polyfill was
  implemented and tested directly against the real facility. A
  reminder that a facility's fidelity scope isn't always fully known
  until implementation, even after a thorough upfront design
  discussion; the project's differential-test discipline is what
  caught it here, not the original design review.
