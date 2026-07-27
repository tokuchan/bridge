\page page_optional Optional

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/optional.hpp`
- `include/deck/cpp17/optional.hpp`
<!-- BRIDGE-DOCS:END -->

TODO: narrative prose for this facility.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `and_then` | function | If opt has a value, invoke f with it and return the result (which must itself be a std::optional); otherwise return an empty result of that same type. | [https://en.cppreference.com/w/cpp/utility/optional](https://en.cppreference.com/w/cpp/utility/optional) |
| `optional` | class | std::optional&lt;T&gt; plus monadic methods, for ecosystems whose std::optional doesn't have them yet. Built on Truss's free functions (include/truss/cpp17/optional.hpp); every other member comes from std::optional&lt;T&gt; via public inheritance. | [https://en.cppreference.com/w/cpp/utility/optional](https://en.cppreference.com/w/cpp/utility/optional) |
| `or_else` | function | If opt has a value, return a copy of it; otherwise invoke f with no arguments and return its result. | [https://en.cppreference.com/w/cpp/utility/optional](https://en.cppreference.com/w/cpp/utility/optional) |
| `transform` | function | If opt has a value, invoke f with it and return std::optional&lt;U&gt; containing the result; otherwise return an empty std::optional&lt;U&gt;. | [https://en.cppreference.com/w/cpp/utility/optional](https://en.cppreference.com/w/cpp/utility/optional) |
<!-- BRIDGE-DOCS:END -->
