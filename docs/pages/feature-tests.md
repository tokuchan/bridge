\page page_feature_tests Feature Tests

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<version>`](https://en.cppreference.com/w/cpp/feature_test) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/rivets/features.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

A Feature Test wraps a single SD-6 feature-test macro (e.g.
`__cpp_lib_optional`) directly, as both a `constexpr` value and an
`#if`-usable macro -- the preferred signal for "does this specific
ecosystem support this specific library feature." Unlike a Detector,
there's no comparator calculus: the macro's value (or `0` if undefined)
already *is* the fact, not a range Rivets computes.

## Example

```cpp
#include <rivets/features.hpp>

constexpr bool has_monadic_optional = bridge::rivets::features::lib_optional >= 202110L;

#if BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL >= 202110L
constexpr bool same_via_macro = true;
#else
constexpr bool same_via_macro = false;
#endif

int main() {
    static_assert(has_monadic_optional == same_via_macro);
}
```

## Notes

- Requires `<version>` included first, or the macro may be invisible
  even on an ecosystem that does support the feature.
- Every Truss/Deck facility that offers real `std::` passthrough
  (`optional`, `expected`, `format`, `print`, `span`) is gated by
  exactly one of these.
- See \ref page_rivets for how Feature Tests relate to Detectors, and
  [ADR-0007](https://github.com/tokuchan/bridge/blob/master/docs/adr/0007-feature-test-wrapping.md)
  for the wrapping rationale.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `BRIDGE_RIVETS_FEATURES_LIB_EXPECTED` | define | `#if`-usable value of `__cpp_lib_expected`, or `0` if undefined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_EXPECTED`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_FORMAT` | define | `#if`-usable value of `__cpp_lib_format`, or `0` if undefined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_FORMAT`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL` | define | `#if`-usable value of `__cpp_lib_optional`, or `0` if undefined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_PRINT` | define | `#if`-usable value of `__cpp_lib_print`, or `0` if undefined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_PRINT`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_SPAN` | define | `#if`-usable value of `__cpp_lib_span`, or `0` if undefined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_SPAN`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_expected` | variable | Value of `__cpp_lib_expected`, or `0` if undefined. | [`<version>::lib_expected`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_format` | variable | Value of `__cpp_lib_format`, or `0` if undefined. | [`<version>::lib_format`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_optional` | variable | Value of `__cpp_lib_optional`, or `0` if undefined. | [`<version>::lib_optional`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_print` | variable | Value of `__cpp_lib_print`, or `0` if undefined. | [`<version>::lib_print`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_span` | variable | Value of `__cpp_lib_span`, or `0` if undefined. | [`<version>::lib_span`](https://en.cppreference.com/w/cpp/feature_test) |
<!-- BRIDGE-DOCS:END -->
