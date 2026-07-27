# Bridge

Bridge the gaps between C++ standards, compilers, and STL implementations. A suite of header-only libraries providing modern C++ features on older standards (down to C++17).

## Language

**Rivets**:
Macro library for detecting language standards, language features, compiler versions, STL implementation versions, and Boost library versions. The fastening/detection layer the other libraries build on.
_Avoid_: Config, feature test, compat macros

**Truss**:
Header-only library providing polyfilled implementations of modern C++ features for older standards, compilers, or STLs that lack them. Three shapes, depending on what (if anything) already exists on older standards to build on: free functions when an STL type already exists (e.g. `bridge::truss::and_then(opt, f)` operating on a plain `std::optional<T>`, never a wrapper type with member methods — that's Deck's job); a complete, from-scratch class when there's no existing type at all (e.g. `bridge::truss::expected<T,E>`, since `std::expected` doesn't exist before C++23); or a complete function-based facility when the real thing is function-shaped rather than type-shaped (e.g. `bridge::truss::format`/`format_to`/`print`, since `std::format` has no pre-existing STL facility to extend before C++20 at all — see [ADR-0012](docs/adr/0012-format-print-truss-owns-the-facility.md)). A Truss-owned class or facility never itself passes through to the real `std::` equivalent, even once available — that selection is Deck's job alone, and applies even when Truss builds one of its own facilities on another of its own facilities (`print` is always built on Truss's own `format`, never on whichever `format` Deck selected). See [ADR-0010](docs/adr/0010-expected-truss-owns-the-class.md). The load-bearing structural layer.
_Avoid_: Polyfills, shims, backports

**Deck**:
Header-only library of higher-level utilities, containers, and algorithms built on top of Truss. Owns the STL-shaped wrapper types consumers actually declare variables as (e.g. `bridge::optional<T>`) — real member methods, matching the target STL interface exactly, selecting per [ADR-0008](docs/adr/0008-best-effort-head-standard.md) whether that's a passthrough alias to the real `std::` type or a wrapper built on Truss's free functions. When Truss owns a full class instead of free functions, that same selection collapses to a plain type alias — Truss's class already has the target shape, so there's nothing left for Deck to wrap (see [ADR-0010](docs/adr/0010-expected-truss-owns-the-class.md)). Not itself a polyfill library — it composes what Truss provides.
_Avoid_: Utils, extras

**Exported namespace**:
Public symbols from both Truss and Deck surface under `bridge::` (e.g. `bridge::optional`, or `bridge::deck::optional`), mirroring `std::` naming conventions rather than exposing `bridge::truss::` / `bridge::deck::` directly for everything.
_Avoid_: bridge_truss_*, per-library-only namespacing

**Detail namespace**:
The implementation namespace mirroring a header's path from its library root (e.g. `bridge::detail::truss::cpp17::optional`); holds the full implementation, including helpers never meant to be public. See [ADR-0001](docs/adr/0001-namespace-and-export-scheme.md).
_Avoid_: impl namespace, internal namespace

**Exports namespace**:
The curated namespace nested inside a Detail namespace holding only the symbols promoted toward the public API (`...::exports`), pulled into `bridge::exports::...` and, from there, hand-picked into `bridge::` itself. See [ADR-0001](docs/adr/0001-namespace-and-export-scheme.md).
_Avoid_: public namespace, api namespace

**Entity**:
One of the four things Rivets detects a version range for: C++ standard, compiler, STL implementation, or Boost. Each Entity gets its own Detector.
_Avoid_: dimension, target

**Detector**:
A seeded per-Entity constant plus five generic parameterized comparators (`gt`/`ge`/`lt`/`le`/`eq`), each in two forms — constexpr functions (`bridge::rivets::gcc::ge(12)`, for `if constexpr`) and function-like macros (`BRIDGE_RIVETS_GCC_GE(12)`, for `#if`) — usable for any number without pre-declaring it. Lives directly in its namespace — no Detail/Exports tiering, since there's never more than one right answer to a version check. See [ADR-0006](docs/adr/0006-detector-naming-calculus.md).
_Avoid_: feature test, version check

**Named Detector**:
A specific, individually-generated `(entity, comparator, number)` check — the sparse, curated cells of the entity × comparator × version table that code actually branches on. Generated via `BRIDGE_RIVETS_DEFINE_DETECTOR` as a zero-arg constexpr alias (`bridge::rivets::gcc::gt_12()`) calling the matching Detector comparator — never a named `#if`-usable macro, since macro expansion cannot emit a `#define`; a named macro form is always one hand-written line. See [ADR-0006](docs/adr/0006-detector-naming-calculus.md).
_Avoid_: feature macro, version macro

**Feature Test**:
A 1:1 wrap of an SD-6 feature-test macro (e.g. `__cpp_lib_optional`), in both an `#if`-usable macro form and a constexpr form, hand-written per feature (never generated, same reason as a Detector's Layer 1). Unlike a Detector, has no comparator calculus — it's already a single curated fact the compiler/stdlib publishes, not a range Rivets computes. The primary, preferred signal for "does this specific ecosystem support this specific library feature"; Detectors serve as a curated override only for known-bad cases. See [ADR-0007](docs/adr/0007-feature-test-wrapping.md).
_Avoid_: feature macro, capability check
