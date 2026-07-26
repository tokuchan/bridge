# Bridge Targets Best-Effort Support for the Head Standard, Per Ecosystem

## Context

A natural-seeming policy for "when do we trust the STL's own
implementation of a feature instead of polyfilling it" would be a single
global language-standard threshold — e.g. "require C++23" for anything
C++23 added. That's the wrong model for how standards actually reach
users: GCC and Clang don't ship every library feature the moment their
language-mode support for a standard lands, and they don't catch up on
the *same* features in the *same* order as each other. A single
"require standard N" gate would either polyfill features an ecosystem
already has natively (wasted work, and a second implementation to keep
correct), or trust features an ecosystem's stdlib hasn't actually shipped
yet (a compile error, or worse, a silent behavioral gap).

## Decision

Bridge's ambition is best-effort support for the **head** standard —
currently C++26 — as a continuously moving target, not a fixed version
bridge waits to formally adopt. For any given feature, bridge introduces
the real STL passthrough **as early as possible for each ecosystem**,
gated on that ecosystem's own evidence (primarily its
[Feature Test](0007-feature-test-wrapping.md), with
[Detectors](0006-detector-naming-calculus.md) as a curated override for
known-bad specific cases) — not on a single global "the language standard
says N" checkpoint. This is the reason Rivets exists in the shape it
does: Detectors and Feature Tests together are the machinery for asking
"does *this* compiler, with *this* stdlib, at *this* version, actually
have it" rather than "does the standard say it should."

Whatever Deck exposes (`bridge::optional`, and future STL-shaped types)
must be behaviorally indistinguishable regardless of which path backed
it — a real passthrough to `std::optional` on one ecosystem, Truss's own
implementation on another. Consumers never see or choose the difference.

## Consequences

- There's no single "bridge now requires C++N" milestone to hit — the
  passthrough boundary moves independently per feature, per ecosystem,
  as each one's Feature Tests report readiness.
- Polyfills don't get deleted just because a new standard nominally
  added the feature; they stay live until every ecosystem bridge cares
  about actually reports it via a Feature Test.
- This raises the bar on Deck's wrapper types: "no detectable difference
  from the user side" means matching the *entire* relevant STL interface
  (comparisons, accessors, everything), not just the specific new methods
  a feature adds.
