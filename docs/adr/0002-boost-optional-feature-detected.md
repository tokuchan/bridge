# Boost Is Optional and Feature-Detected

## Context

Truss and Deck may want to lean on Boost for some implementations rather
than reinventing them. But Truss's baseline promise is that it works on
nothing but the C++17 standard library — a hard Boost dependency would
break that promise for consumers who don't have Boost available.

## Decision

A CMake option, `BRIDGE_WITH_BOOST` (`AUTO`/`ON`/`OFF`, default `AUTO`),
gates whether Boost-backed implementations are compiled in. `AUTO` uses
Boost if `find_package(Boost)` succeeds; `OFF` disables it unconditionally;
`ON` hard-fails configure if Boost isn't found. When Boost isn't in use,
Truss/Deck fall back to their own implementations — Rivets is responsible
for detecting Boost's presence/version so headers can branch on it via
`BRIDGE_HAS_BOOST` and friends (see
[ADR-0001](0001-namespace-and-export-scheme.md) for how the resulting
symbol gets selected at the `bridge::`/`bridge::<lib>::` promotion step).

## Considered Options

- **Required for Deck only.** Rejected — it would make every Deck header
  unusable without Boost, contradicting Deck's role as general-purpose
  header-only utilities.
- **Required project-wide.** Rejected — it would make Rivets' own
  Boost-version-detection moot and shuts out any C++17-only, Boost-free
  consumer, which is exactly the audience Truss exists to serve.
