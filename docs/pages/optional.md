\page page_optional Optional

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<optional>`](https://en.cppreference.com/w/cpp/utility/optional) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/optional.hpp`
- `include/deck/cpp17/optional.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::optional<T>` is `std::optional<T>` plus three monadic
methods: `and_then`, `or_else`, and `transform`. C++23 adds these
three methods to `std::optional<T>`. `bridge::optional<T>` gives you
these same three methods on every C++ standard bridge supports.

`std::optional<T>` itself is part of C++17. Truss builds only the
three monadic methods, as free functions on a real `std::optional<T>`.
Deck then makes a choice for each ecosystem. When the ecosystem's own
`std::optional` already has the three methods, Deck's choice is
passthrough: `bridge::optional<T>` is `std::optional<T>` directly.
When the ecosystem's own `std::optional` does not have the three
methods yet, Deck's choice is the polyfill: `bridge::optional<T>` is
a class that inherits from `std::optional<T>` and adds the three
methods.

## Example

```cpp
#include <deck/cpp17/optional.hpp>
#include <string>
#include <cassert>

int main() {
    bridge::optional<std::string> name{"hello"};

    auto result = name.transform([](const std::string& s) { return s.size(); })
                      .and_then([](std::size_t n) { return bridge::optional<std::size_t>{n * 2}; });

    assert(result.has_value());
    assert(*result == 10);
}
```

## Divergences

None. Under passthrough, `bridge::optional<T>` is `std::optional<T>`,
with no change at all. Under the polyfill, `bridge::optional<T>` adds
only the three monadic methods. `std::optional<T>` itself supplies
every other method, unchanged. This includes `emplace`, `reset`, and
the comparison operators.

## Passthrough

Deck checks one Feature Test:
`BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL >= 202110L`. This is the Feature
Test value for `std::optional`'s monadic methods
([P0798](https://en.cppreference.com/w/cpp/feature_test)). Deck
chooses passthrough when this check passes. Deck chooses the polyfill
when this check fails.

See [ADR-0008](https://github.com/tokuchan/bridge/blob/master/docs/adr/0008-best-effort-head-standard.md)
for the bar this choice must clear. See CONTEXT.md for how this shape
compares with `expected`'s shape and `format`'s shape. `expected` is a
full class that Truss owns outright. `format` is a complete
function-based facility. `optional` only adds free functions onto an
existing STL type.

The ADR and CONTEXT.md links above point at the repository on GitHub,
not this generated site. Doxygen does not read them from here.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `and_then` | function | If `opt` holds a value, this function calls `f` with the value. If `opt` holds no value, this function does not call `f`. | [`<optional>::and_then`](https://en.cppreference.com/w/cpp/utility/optional) |
| `optional` | class | This class is a polyfill. It adds monadic methods to `std::optional<T>`, for an ecosystem whose `std::optional` does not have them yet. | [`<optional>::optional`](https://en.cppreference.com/w/cpp/utility/optional) |
| `or_else` | function | If `opt` holds a value, this function returns a copy of `opt`. If `opt` holds no value, this function calls `f` and returns `f`'s result. | [`<optional>::or_else`](https://en.cppreference.com/w/cpp/utility/optional) |
| `transform` | function | If `opt` holds a value, this function calls `f` with the value and returns the result in a new `std::optional<U>`. If `opt` holds no value, this function returns an empty `std::optional<U>`. | [`<optional>::transform`](https://en.cppreference.com/w/cpp/utility/optional) |
<!-- BRIDGE-DOCS:END -->
