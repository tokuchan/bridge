# Detector Naming Calculus: Generic Primitives + Curated Named Detectors

## Context

Rivets needs to detect four independent things — C++ standard, compiler,
STL implementation, Boost version — each as a version *range* (not just a
single point), and expose those checks so Truss/Deck can select or guard
implementations. Two shapes were rejected outright:

- **Pure `#ifdef`/`#endif`** works, but scatters raw preprocessor
  conditionals through every consuming header, with no shared vocabulary
  for what's being checked or why.
- **One hand-written comparator per (entity, comparator, number) triple
  someone happens to need**, with no generative structure, would grow
  without bound and duplicate the same five lines of boilerplate forever.

Conceptually, every possible `(entity, comparator, number)` triple (plus
two-bound ranges) forms a Cartesian product. The vast majority of that
table is never queried by any real code — only a sparse, specific set of
thresholds actually matter (a bug fixed in GCC 12, a feature added in
Boost 1.87, and so on). The naming scheme needs to name *exactly those
entries*, consistently, without pre-declaring the whole table.

## Decision

Two layers.

### Layer 1 — Detector (one per entity, hand-written once)

A seeded constant, computed once from a small number of raw
compiler/library macros (`__GNUC__`, `BOOST_VERSION`, etc.), plus five
generic parameterized comparators in **two forms**, both taking `n` as a
real argument so neither needs `n` pre-declared anywhere:

- `gt(n)`, `ge(n)`, `lt(n)`, `le(n)`, `eq(n)` — constexpr functions in
  `bridge::rivets::<entity>` (e.g. `bridge::rivets::gcc::ge(12)`), usable
  directly in `if constexpr`.
- `BRIDGE_RIVETS_<ENTITY>_GT(n)`, `_GE(n)`, `_LT(n)`, `_LE(n)`, `_EQ(n)` —
  **function-like** preprocessor macros (e.g.
  `BRIDGE_RIVETS_GCC_GE(12)`), the `#if`-usable sibling. No `ne` in
  either form; a version check is never naturally "not exactly X."

Both forms exist because neither can substitute for the other: `#if`
cannot call a constexpr function, and `if constexpr` cannot depend on a
value that's only ever a `#define`. Function-like macros need an
argument to invoke — `BRIDGE_RIVETS_GCC_GE(12)` — which is what makes
them work for *any* `n` in `#if` without pre-declaring anything, exactly
like the constexpr comparators.

The raw seeded value itself (`BRIDGE_RIVETS_GCC_VERSION`) is also
`#if`-usable directly, for the rare case that needs to gate genuinely
invalid syntax, which `if constexpr` cannot do — `if constexpr` can only
choose between branches that are each independently valid C++; it cannot
make an unparseable branch (a missing keyword, a header that doesn't
exist) disappear the way `#ifdef` can.

### Layer 2 — Named Detector (generated on demand)

The sparse, curated table entries: a specific `(entity, comparator,
number)` check worth a self-documenting name at its call sites. A
generator macro, `BRIDGE_RIVETS_DEFINE_DETECTOR(entity, cmp, n)`,
produces **only** a zero-arg constexpr alias —
`bridge::rivets::<entity>::<cmp>_<n>()`, calling the matching Layer-1
comparator — because that expands to an ordinary C++ declaration, which
macro expansion can legally produce.

It does **not** also produce a named `#if`-usable macro
(`BRIDGE_RIVETS_GCC_GT_12`), and it structurally cannot: the C++ standard
says a macro's fully-expanded replacement text "is not processed as a
preprocessing directive even if it resembles one" ([cpp.rescan]). No
macro invocation — this generator or any other — can emit a `#define` as
a side effect. When a named `#if` token is genuinely wanted, it's one
hand-written line: `#define BRIDGE_RIVETS_GCC_GT_12
BRIDGE_RIVETS_GCC_GT(12)`. That line isn't a fallback for an awkward
case; it's the *only* mechanism by which any named object-like macro
ever comes to exist.

A range takes two comparator/number pairs and concatenates both bounds in
name order: `BRIDGE_RIVETS_DEFINE_DETECTOR(gcc, ge, 10, lt, 13)` →
`bridge::rivets::gcc::ge_10_lt_13()`.

Boost is the one entity whose number is naturally two-component
(major.minor, e.g. `ge_1_87`, matching how Boost versions are actually
discussed) rather than the single major-version int every other entity
uses; the generator has a distinct form for that shape.

### Namespace: `bridge::rivets::`, not `bridge::`

Unlike Truss/Deck (see [ADR-0001](0001-namespace-and-export-scheme.md)),
detectors do not go through the `detail` → `exports` → `bridge::`
promotion tiers. That machinery exists to pick among *multiple candidate
implementations* of the same symbol (Truss's own polyfill vs. Boost vs.
`std::` passthrough) — there is never more than one right answer to "is
this GCC 12 or later," so there's nothing to select among, and the tiers
would be pure ceremony.

## What the generator can't do

Macro expansion cannot emit a `#define`. This isn't a corner case to work
around — it's a hard rule of the language ([cpp.rescan]) — so a named,
`#if`-usable macro for a specific Named Detector is always one
hand-written line, never something the generator produces. This is an
expected, not exceptional, path: the calculus is documented in full at
the top of `include/rivets/detail/detector.hpp` specifically so a
hand-written Detector or Named Detector stays consistent with the
generated ones.

## Consequences

- Every new entity needs one Layer-1 header (seed + five comparators);
  Named Detectors for that entity cost one generator line each, added
  only when something actually branches on that threshold.
- The `standard` entity's Layer-1 comparators take the short ordinal
  people actually say (`ge(20)` for C++20), translated internally via the
  existing `BRIDGE_CPP17`/`BRIDGE_CPP20`/`BRIDGE_CPP23` constants rather
  than inventing a second encoding.
