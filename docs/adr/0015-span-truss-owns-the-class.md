# For `span`, Truss Owns a Full Class, with Both Extents

## Context

`std::span<T, Extent>` doesn't exist at all before C++20 — like
`expected` ([ADR-0010](0010-expected-truss-owns-the-class.md)), there's
no pre-existing STL type from an older standard for Truss to attach
free functions onto. Unlike `expected`, `span` has no monadic surface
to speak of; its complexity is elsewhere — a compile-time `Extent`
parameter that changes the type's storage layout, a range-constructing
constructor whose real gate is a set of C++20 concepts, and a handful
of member functions (`first`/`last`/`subspan`) whose *return type*
itself depends on whether `Extent` is known at compile time.

## Decision

Truss owns a complete, from-scratch `span<T, Extent = dynamic_extent>`
class (following `expected`'s precedent: a full class when there's no
existing type to extend), plus `dynamic_extent` and the free functions
`as_bytes`/`as_writable_bytes`. `bridge::truss::span<T, Extent>` is
**unconditionally** Truss's own class, regardless of standard or
toolchain — Truss never passes through, even under C++20/23 where the
real type is available. Passthrough selection happens exactly once, in
Deck: `bridge::deck::span<T, Extent>` (promoted to `bridge::span`) is a
plain type alias to `std::span<T, Extent>` when
`BRIDGE_RIVETS_FEATURES_LIB_SPAN` (`__cpp_lib_span`) confirms it, or to
`bridge::truss::span<T, Extent>` otherwise — the same shape
`expected`'s Deck header uses, since Truss's class already has the
target shape and there's nothing left for Deck to wrap.

Empirically confirmed via direct compiler probe before implementation
(not assumed): `__cpp_lib_span` is undefined under `-std=c++17` and
exactly `202002L` (the P0122R7 paper value) from `-std=c++20` onward on
both GCC 15.3 and Clang 20, including under `-std=c++23` — this
libstdc++ hasn't yet bumped the macro to `202311L` for the C++23
tuple-like interface (see Scope below).

### Fidelity scope

Per [ADR-0008](0008-best-effort-head-standard.md)'s "behaviorally
indistinguishable" bar:

- **Matched, in full**: both static and dynamic `Extent`. A
  fixed-`Extent` span stores no runtime size at all — confirmed via
  probe that real `sizeof(std::span<int,4>)` is half
  `sizeof(std::span<int>)` on this project's compiler pair — and the
  polyfill's storage layout matches. Static-to-dynamic conversion is
  implicit; dynamic-to-static is constructible but *not* implicitly
  convertible — both confirmed via `static_assert`/`is_convertible_v`
  probes against real `std::span`, not assumed from the standard's
  prose alone. `first`/`last`/`subspan`'s dual return-type behavior is
  matched exactly: the *template* forms (`first<Count>()`,
  `subspan<Offset, Count>()`) on a fixed-extent span return a new
  fixed-extent span when the result size is known at compile time; the
  *runtime* forms (`first(n)`, `last(n)`, `subspan(offset, count)`)
  always return `span<T, dynamic_extent>`, even when called on a
  fixed-extent span. Confirmed via `static_assert` against real
  `std::span` in each case, not assumed from the standard's prose
  alone.

  The converting constructor's `explicit`-ness needed its own
  probe-then-fix cycle: real `std::span` is
  `explicit(Extent != dynamic_extent && OtherExtent == dynamic_extent)`
  (explicit only when narrowing from an unknown-at-compile-time source
  extent to a known one), which needs
  C++20's conditional `explicit(bool)` to express directly. Reproduced
  exactly — not an `expected`-style always-explicit fallback — via two
  separate constructor templates, one plain and one `explicit`, each
  SFINAE-enabled on the opposite half of the same condition; found
  during implementation that the SFINAE has to live on an extra
  defaulted *function* parameter rather than a defaulted template type
  parameter, since two constructor templates with an otherwise-identical
  template-parameter-list shape are rejected as "cannot be overloaded"
  even when their `enable_if` conditions differ (hit that exact
  compiler error before settling on this shape, not assumed).
  Likewise for
  `std::span` for both forms before writing the polyfill's own
  versions. The default constructor is conditionally available only
  when `Extent == dynamic_extent || Extent == 0`, matching the real
  type's constraint (`std::is_default_constructible_v` probed both
  ways against real `std::span<int,3>` and `std::span<int,0>`).
- **Named-case construction, not generic SFINAE**: the range/container-
  constructing constructor and deduction guides are implemented for
  the specific cases real code overwhelmingly uses — a pointer+count,
  a C array, `std::array` (mutable and const element type), `std::vector`,
  `std::string` — each verified individually against real `std::span`,
  rather than a single duck-typed "anything with `.data()`+`.size()`"
  constructor approximating `ranges::contiguous_range`+`sized_range`.
  **Not a divergence-note case** ([ADR-0011](0011-warn-on-surprising-facility-divergences.md)
  reviewed and deliberately not applied): a container type outside the
  named list fails to *compile* under the polyfill, exactly the kind of
  loud, immediate, compile-time-visible gap real `std::span` itself
  produces for a genuinely non-contiguous-range argument — not the
  silent runtime-behavior surprise `BRIDGE_RIVETS_DIVERGENCE_NOTE`
  exists for. Mirrors `expected`'s own "more conservative, fails loud,
  no note needed" precedent (`ADR-0010`'s fidelity-scope section).
- **`as_bytes`/`as_writable_bytes`: in scope**, mechanical
  `reinterpret_cast`-based reinterpretation of a span's elements as
  `std::byte`, element-count math (`size() * sizeof(T)`) confirmed via
  probe against real `std::span` before writing the polyfill's version.
- **Explicitly out of scope**: the C++23 tuple-like interface
  (`get<I>(span)`, `std::tuple_size`/`std::tuple_element` for a
  fixed-extent span) — a genuinely separate feature bumping
  `__cpp_lib_span` to `202311L`, the same "two thresholds in one
  facility" shape `format`/`print` had
  ([ADR-0012](0012-format-print-truss-owns-the-facility.md)). Deferred
  as a disclosed follow-up rather than pulled into this pass, for the
  same scope-discipline reason `format`/`print` trimmed range/chrono/
  locale formatting. No compiler in this project's matrix reports
  `202311L` yet, so the follow-up's actual gate value needs its own
  empirical confirmation when that work begins, not an assumption that
  the paper's documented value is what real toolchains will report
  (the same caution `format`'s own Feature Test threshold noted).

## Consequences

- Because Truss never passes through, `bridge::truss::span<T,Extent>`
  and `std::span<T,Extent>` are two distinct types in any C++20 build
  — the test suite exploits this directly with a differential test
  comparing both types' traits and behavior in the same translation
  unit, same technique `expected`'s own differential test uses.
- A consumer relying on a custom contiguous-range type outside the
  named-case list (pointer+count, C array, `std::array`, `std::vector`,
  `std::string`) to construct a `bridge::span` under the polyfill path
  will hit a compile error there that real `std::span` (under
  passthrough) would not — a disclosed, compile-time-visible gap, not
  a silent behavioral difference.
- The next facility whose scope naturally splits into two
  Feature-Test-gated thresholds (like `format`/`print`, now `span`'s
  own core-vs-tuple-like-interface split) documents the deferred half
  explicitly, with its own gate value confirmed empirically when that
  work actually begins, rather than assumed from the paper's
  documented value.
