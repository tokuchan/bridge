\page page_optional Optional

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<optional>`](https://en.cppreference.com/w/cpp/utility/optional) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/optional.hpp`
- `include/deck/cpp17/optional.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::optional<T>` is `std::optional<T>` plus its C++23 monadic
methods (`and_then`, `or_else`, `transform`), available regardless of
which standard is actually active. `std::optional<T>` itself has
existed since C++17, so Truss builds only the three monadic methods as
free functions on a real `std::optional<T>`; Deck wraps them into a
thin class publicly inheriting `std::optional<T>` when the ecosystem's
own `std::optional` doesn't have them yet, or passes through directly
when it does.

## Example

```cpp
#include <deck/cpp17/optional.hpp>

bridge::optional<std::string> name{"hello"};

auto result = name.transform([](const std::string& s) { return s.size(); })
                  .and_then([](std::size_t n) { return bridge::optional<std::size_t>{n * 2}; });

// result == bridge::optional<std::size_t>{10}
```

## Divergences

None. `bridge::optional<T>` matches `std::optional<T>` exactly under
passthrough, and the polyfill path adds only the three monadic methods
on top of public inheritance from the real `std::optional<T>` --
everything else (comparisons, accessors, `emplace`, `reset`, ...) is
`std::optional<T>`'s own behavior, unmodified.

## Passthrough

Deck selects between passthrough and polyfill on
`BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL >= 202110L` (the Feature Test
value for `std::optional`'s monadic methods, [P0798](https://en.cppreference.com/w/cpp/feature_test)).
See [ADR-0008](https://github.com/tokuchan/bridge/blob/master/docs/adr/0008-best-effort-head-standard.md)
for the "behaviorally indistinguishable" bar this selection has to
clear, and CONTEXT.md for how this shape (free functions added onto an
existing STL type) differs from `expected`'s (a full class Truss owns
outright) and `format`'s (a complete function-based facility). (ADR and
CONTEXT.md links here point at the repository on GitHub, not this
generated site -- they aren't part of Doxygen's input.)

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `and_then` | function | If `opt` has a value, invoke `f` with it and return the result (which must itself be a `std::optional`); otherwise return an empty result of that same type. | [`<optional>::and_then`](https://en.cppreference.com/w/cpp/utility/optional) |
| `optional` | class | `std::optional<T>` plus monadic methods, for ecosystems whose `std::optional` doesn't have them yet. Built on Truss's free functions (include/truss/cpp17/optional.hpp); every other member comes from `std::optional<T>` via public inheritance. | [`<optional>::optional`](https://en.cppreference.com/w/cpp/utility/optional) |
| `or_else` | function | If `opt` has a value, return a copy of it; otherwise invoke `f` with no arguments and return its result. | [`<optional>::or_else`](https://en.cppreference.com/w/cpp/utility/optional) |
| `transform` | function | If `opt` has a value, invoke `f` with it and return `std::optional<U>` containing the result; otherwise return an empty `std::optional<U>`. | [`<optional>::transform`](https://en.cppreference.com/w/cpp/utility/optional) |
<!-- BRIDGE-DOCS:END -->
