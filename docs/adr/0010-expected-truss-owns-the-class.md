# For `expected`, Truss Owns a Full Class, Not Free Functions

## Context

Truss's established shape (`0001`, and the `optional` free-function
precedent) is: Truss adds polyfilled *behavior* onto an STL type that
already exists on older standards, as free functions; Deck owns the
STL-shaped wrapper type consumers actually declare variables as,
choosing per [ADR-0008](0008-best-effort-head-standard.md) whether
that wrapper is a passthrough alias or built on Truss's functions.

`std::expected<T,E>` breaks the precondition that shape depends on:
it doesn't exist at all before C++23. There is no pre-existing
`std::expected` for Truss to attach `and_then`/`or_else` onto for
C++17/20 — the type itself, not just its monadic surface, is missing.

## Decision

Truss owns a complete, from-scratch `expected<T,E>` class
implementation (constructors, assignment, observers, `emplace`,
`swap`, comparisons, and the full monadic surface — `and_then`,
`or_else`, `transform`, `transform_error`, `error_or`) for the
pre-C++23 case, plus `unexpected<E>`, `bad_expected_access<E>`, and
`unexpect_t`. This is a second refinement of the Truss/Deck boundary:
Truss provides free functions when there's an existing STL type to
extend, and a full class when there isn't.

`bridge::truss::expected<T,E>` is **unconditionally** Truss's own
class, regardless of standard or toolchain — Truss never passes
through to `std::expected` itself, even when compiling under C++23
where the real type is available. Passthrough selection happens
exactly once, in Deck: `bridge::deck::expected<T,E>` (promoted to
`bridge::expected`) is an alias to `std::expected<T,E>` when the
`__cpp_lib_expected` [Feature Test](0007-feature-test-wrapping.md)
confirms it, or to `bridge::truss::expected<T,E>` otherwise. This
keeps each library's job singular — Truss polyfills, Deck selects —
and means the polyfill and the real type coexist as distinct types in
any C++23 build, which is what makes a direct differential trait test
possible (see Consequences).

Detection is one new Feature Test, `__cpp_lib_expected`, added to the
existing `include/rivets/features.hpp`. Empirical verification (real
compiles against this project's own GCC/Clang devShells, not
secondary sources) found `error_or` — a member cppreference documents
but which the original P0323R12 paper explicitly excluded ("not in
this proposal"; it arrived by the time both compilers' libstdc++
shipped `__cpp_lib_expected`) works correctly under the *same*
`__cpp_lib_expected` value (`202211L`) as the rest of the type, on
both compilers tested. No separate Feature Test or Detector override
is introduced for `error_or` specifically; the extension point
(a Detector override, same pattern as `optional`'s Boost case) stays
documented but unbuilt until a real ecosystem disagrees.

### Fidelity scope

[ADR-0008](0008-best-effort-head-standard.md) requires Deck's exposed
type to be "behaviorally indistinguishable" regardless of backing
path. For `expected`, that requirement has trait-level teeth beyond
value semantics:

- **Matched**: `std::expected`'s copy/move-assignment operators (and
  several constructors) are conditionally deleted, gated on
  `is_nothrow_move_constructible_v<T/E>` plus copy-assignable/
  constructible conditions. Truss's polyfill replicates these exact
  conditions. This is the one divergence category worth the cost:
  without it, code compiling fine against the C++17 polyfill could
  silently fail to compile the moment a consumer's toolchain crosses
  the passthrough threshold and gets the real `std::expected` instead
  — a portability trap, not just a cosmetic difference.
- **Best-effort**: `noexcept` specifications, matched where cheap.
- **Explicitly out of scope**: conditional triviality (trivially
  copyable/destructible when `T`/`E` are). Replicating this needs the
  same base-class-specialization machinery real STL implementations
  use to get conditional triviality, which is a large scope increase
  not justified for a header-only backport. Accepted as a known,
  documented limitation — a consumer relying on
  `std::is_trivially_copyable_v` will observe a difference between
  the polyfill and passthrough paths.
- **Exception safety**: assignment uses straightforward
  destroy-then-construct, not the standard's full two-stage
  "reinit-expected" technique for the narrow case where a throwing
  move constructor could otherwise leave the object valueless after a
  failed reassignment. Also an accepted, documented simplification.

## Consequences

- `CONTEXT.md`'s Truss and Deck entries are updated for this second
  refinement: Truss's entry gains "a full class when there's no
  existing STL type to extend"; Deck's entry gains "for a Truss-owned
  class, that selection is just a type alias — there's nothing left
  for Deck to wrap."
- Because Truss never passes through, `bridge::truss::expected<T,E>`
  and `std::expected<T,E>` are two distinct types in any C++23 build.
  The test suite exploits this directly: a differential test
  `static_assert`s matching traits (e.g. `is_copy_assignable_v`) of
  both types against each other in the same translation unit, rather
  than testing each path in isolation and hoping they agree.
- The next class-shaped feature that doesn't have a pre-existing STL
  type to extend follows this same pattern by default: Truss owns the
  class, Deck aliases, fidelity scope gets the same explicit
  matched/best-effort/out-of-scope breakdown as above rather than an
  unstated assumption of full conformance.
