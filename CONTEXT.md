# Bridge

Bridge the gaps between C++ standards, compilers, and STL implementations. A suite of header-only libraries providing modern C++ features on older standards (down to C++17).

## Language

**Rivets**:
Macro library for detecting language standards, language features, compiler versions, STL implementation versions, and Boost library versions. The fastening/detection layer the other libraries build on.
_Avoid_: Config, feature test, compat macros

**Truss**:
Header-only library providing polyfilled implementations of modern C++ features for older standards, compilers, or STLs that lack them (e.g. a C++17-compatible `optional` with monadic methods). The load-bearing structural layer — mirrors STL naming and shape.
_Avoid_: Polyfills, shims, backports

**Deck**:
Header-only library of higher-level utilities, containers, and algorithms built on top of Truss, assuming Truss's polyfills are available. Not itself a polyfill library.
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
