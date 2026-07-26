# Feature Test Wrapping: A New Rivets Concept Distinct from Detector

## Context

Truss needs to know whether the *stdlib actually shipped* a specific
library feature (e.g. `std::optional`'s monadic methods) before deciding
whether to polyfill it. The compiler/standard/STL Detectors built in
[ADR-0006](0006-detector-naming-calculus.md) answer a related but
different question — "is the compiler/standard/STL at least version
N?" — which is a weaker signal: a conforming C++23 compiler doesn't
guarantee its *specific stdlib* has caught up on every library feature
the standard added, and different STL implementations catch up at
different rates for different features. The C++ committee already
publishes a precise mechanism for exactly this: SD-6 feature-test macros
(`__cpp_lib_optional` and friends), which vendors are expected to bump
only once they've actually shipped the feature.

## Decision

Rivets wraps each feature-test macro 1:1, in both an `#if`-usable macro
form and a constexpr form — e.g. `BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL`
and `bridge::rivets::features::lib_optional`. Both forms exist
independently and are **hand-written per feature**, not generated, for
the identical [cpp.rescan] reason a Detector's Layer 1 is hand-written
(see ADR-0006): a macro invocation can never emit a `#define`, so no
generator could produce the `#ifdef` block each wrap needs.

A **Feature Test** is a different kind of thing from a **Detector**:

- A Detector computes a range check (`gt`/`ge`/`lt`/`le`/`eq`) over a
  version rivets seeds itself from a raw macro.
- A Feature Test has no comparator calculus at all — it's already a
  single curated fact (a value, or absence) the compiler/stdlib
  publishes directly. Rivets wraps it; it doesn't compute anything.

Rivets does **not** interpret what a Feature Test's value means for any
particular consumer (e.g. that `202110L` is the threshold for monadic
`optional` support) — that interpretation is domain knowledge specific
to whatever's consuming it (Truss, in this case), not a generic
detection concern.

**Precedence when both exist**: the Feature Test is the primary,
preferred signal. Rivets' compiler/STL Detectors are reserved as a
curated *override* for the sparse cases where a specific ecosystem's
Feature Test macro is missing entirely or is known to be wrong (ships
the macro claiming support, but the implementation has a bug) — matching
the same "sparse, curated table" philosophy Named Detectors already use.
No such override exists yet for `optional`; the extension point is
documented, not built ahead of a real need.

## Consequences

- Every new Feature Test rivets wraps costs one small hand-written block
  (mirroring the pattern already established for Detector Entities),
  not a generator invocation.
- Truss/Deck headers that need a Feature Test's value at `#if` time (to
  choose between two different type definitions, not just branch inside
  a function body) use the macro form; code that only needs a runtime-ish
  compile-time value uses the constexpr form — same dual-form reasoning
  as Detectors.
