\page page_optional Optional

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<optional>`](https://en.cppreference.com/w/cpp/utility/optional) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/optional.hpp`
- `include/deck/cpp17/optional.hpp`
<!-- BRIDGE-DOCS:END -->

`std::optional<T>` itself has existed since C++17, so unlike `expected`
or `format`, Truss has nothing to build from scratch here -- Truss owns
only the *monadic* methods (`and_then`, `or_else`, `transform`) that
arrived later, in C++23, as plain free functions operating on a real
`std::optional<T>` (`bridge::truss::and_then(opt, f)`, never a wrapper
type). Deck then owns `bridge::optional<T>`: a thin class publicly
inheriting `std::optional<T>` and adding those same three operations as
real member methods, so it reads and behaves exactly like the C++23
type regardless of which standard is actually active underneath. See
[ADR-0008](https://github.com/tokuchan/bridge/blob/master/docs/adr/0008-best-effort-head-standard.md)
for the "behaviorally indistinguishable" bar this has to clear, and
CONTEXT.md for how this shape (free functions on an existing STL type)
differs from `expected`'s (a full class Truss owns outright) and
`format`'s (a complete function-based facility) -- three different
answers to the same underlying question, "what does Truss actually
need to build." (ADR and CONTEXT.md links here point at the repository
on GitHub, not this generated site -- they aren't part of Doxygen's
input.)

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `and_then` | function | If `opt` has a value, invoke `f` with it and return the result (which must itself be a `std::optional`); otherwise return an empty result of that same type. | [`<optional>::and_then`](https://en.cppreference.com/w/cpp/utility/optional) |
| `optional` | class | `std::optional<T>` plus monadic methods, for ecosystems whose `std::optional` doesn't have them yet. Built on Truss's free functions (include/truss/cpp17/optional.hpp); every other member comes from `std::optional<T>` via public inheritance. | [`<optional>::optional`](https://en.cppreference.com/w/cpp/utility/optional) |
| `or_else` | function | If `opt` has a value, return a copy of it; otherwise invoke `f` with no arguments and return its result. | [`<optional>::or_else`](https://en.cppreference.com/w/cpp/utility/optional) |
| `transform` | function | If `opt` has a value, invoke `f` with it and return `std::optional<U>` containing the result; otherwise return an empty `std::optional<U>`. | [`<optional>::transform`](https://en.cppreference.com/w/cpp/utility/optional) |
<!-- BRIDGE-DOCS:END -->
