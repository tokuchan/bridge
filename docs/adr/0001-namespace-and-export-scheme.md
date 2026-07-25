# Namespace and Export Scheme

## Context

Truss, Deck, and Rivets need to provide the same public symbol (e.g. `bridge::optional`) from different backing implementations, selected based on detected compiler, language standard, and Boost availability — while keeping implementation details out of the public API surface, and keeping headers documentable/navigable by mirroring their file path in their namespace.

## Decision

Every header defines its implementation inside `bridge::detail::<library>::<standard>::<header-name>`, mirroring the file's path from the library root (e.g. `include/truss/cpp17/optional.hpp` → `bridge::detail::truss::cpp17::optional`).

Symbols meant for public consumption are placed in a nested `::exports` namespace inside that (`bridge::detail::truss::cpp17::optional::exports`). These are pulled via `using namespace` into `bridge::exports::truss::cpp17::optional`, where the `<standard>` and header-name segments are declared `inline` — collapsing away the version-specific segment, since only one standard-specific variant is ever compiled per translation unit. This makes the same exports reachable as `bridge::exports::truss::optional`.

The `<library>` segment (`truss`/`deck`) is deliberately **not** inline — it stays explicit at the `exports` level.

Finally, both `bridge::<library>::<header-name>` (e.g. `bridge::deck::optional`) and the flattened `bridge::<header-name>` (e.g. `bridge::optional`) are hand-written, symbol-by-symbol `using` declarations — not a blanket `using namespace`. This is what lets `bridge::optional` resolve to Truss's own polyfill, a Boost-backed implementation, or a direct alias to `std::optional`, chosen per compiler/language-standard/Boost-availability, without ambiguity between candidate sources.

## Consequences

- Every new header sets up the same 4-tier structure (`detail` → `detail::...::exports` → `exports` → hand-picked `using`). Mechanical but boilerplate-heavy; worth scripting/codegen if it becomes tedious.
- The manual `using`-declaration step at the `bridge::`/`bridge::<library>::` level is the single point where cross-source selection logic (own polyfill vs. Boost vs. `std` passthrough) lives, driven by Rivets' feature-detection macros.
