\page page_rivets Rivets: the sparse Cartesian product

Rivets is bridge's detection layer: it answers "what does this specific
build actually have" so Truss and Deck can make a passthrough-or-polyfill
decision, or a consumer can gate their own code the same way. It has no
`std::` equivalent to mirror -- it's a bridge-only concept -- which is
why it gets a hand-written overview page instead of a generated one:
there's no single cppreference page to anchor a facility table to the
way \subpage page_optional or \subpage page_expected can.

Three facilities make up Rivets:

- \subpage page_detectors -- version-range checks against a compiler,
  standard-library implementation, the active C++ standard, or Boost.
- \subpage page_feature_tests -- 1:1 wraps of SD-6 feature-test macros
  (`__cpp_lib_optional` and friends).
- \subpage page_diagnostics -- the `BRIDGE_RIVETS_DIVERGENCE_NOTE`
  compiler-visible-note mechanism.

## The mental model: a sparse Cartesian product

A Detector is really a coordinate in a three-dimensional space: **Entity**
(compiler, standard-library implementation, language standard, or Boost)
x **Comparator** (`gt`/`ge`/`lt`/`le`/`eq`) x **version number**. That
space is enormous and almost entirely uninteresting -- nobody has ever
needed to ask "is this GCC greater than version 4,017" -- so Rivets
doesn't try to populate it. Layer 1 (the five generic comparators per
Entity, e.g. `bridge::rivets::gcc::ge(int)`) makes the *entire* space
reachable with a single, parameterized function per comparator. Layer 2
(a Named Detector, e.g. `bridge::rivets::gcc::ge_13()`, generated via
`BRIDGE_RIVETS_DEFINE_DETECTOR`) then populates just the sparse handful
of specific `(Entity, Comparator, version)` cells that code in this
codebase (or a consumer's) actually branches on -- a curated subset of
the product, not an attempt to enumerate it.

Feature Tests sit next to this, not inside it: where a Detector answers
"what version is this," a Feature Test answers "does this specific
library feature exist at all" by wrapping the standard's own SD-6
feature-test macro directly. It's the preferred signal whenever one
exists; Detectors are the fallback for cases a Feature Test can't cover
(a known-bad implementation that defines the macro but ships a broken
feature, for instance).

## Why both a `constexpr` form and a `#if`-usable macro form

Every Detector and Feature Test comes in two shapes doing the same
check: a `constexpr` function/variable (`bridge::rivets::gcc::ge(13)`,
`bridge::rivets::features::lib_format`) for `if constexpr` and other
compile-time-but-not-preprocessor contexts, and a function-like macro
(`BRIDGE_RIVETS_GCC_GE(13)`, `BRIDGE_RIVETS_FEATURES_LIB_FORMAT`) for
`#if`. Neither one subsumes the other: a `constexpr` value can't gate a
`#include` or an `#if`-guarded block that must not even be *parsed* on
some configurations (a header that doesn't exist yet, a type that isn't
declared), while a macro can't participate in ordinary C++ overload
resolution, template metaprogramming, or anywhere else a real value is
more natural to write than a preprocessor conditional. Both bridge's own
code and a consumer's may need either, depending on which kind of gate
they're writing -- so both exist, for every Detector and Feature Test,
rather than picking one and asking users to work around the other.
