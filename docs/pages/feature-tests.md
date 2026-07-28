\page page_feature_tests Feature Tests

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<version>`](https://en.cppreference.com/w/cpp/feature_test) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/rivets/features.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

A Feature Test wraps one SD-6 feature-test macro directly, for
example `__cpp_lib_optional`. A Feature Test gives you this macro's
value in two forms: a `constexpr` value, and an `#if`-usable macro. A
Feature Test is the preferred signal for one question: does this
specific ecosystem support this specific library feature?

A Feature Test is different from a Detector. A Feature Test has no
comparator calculus. The macro's value already is the fact. This
value is `0` when the macro is not defined. Rivets does not compute
this value as a range, the way a Detector does.

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

- This facility needs `<version>` included first. Without this, the
  macro may stay invisible, even on an ecosystem that supports the
  feature.
- Every Truss/Deck facility that offers real `std::` passthrough is
  gated by exactly one Feature Test. This includes `optional`,
  `expected`, `format`, `print`, and `span`.
- See \ref page_rivets for how Feature Tests relate to Detectors. See
  [ADR-0007](https://github.com/tokuchan/bridge/blob/master/docs/adr/0007-feature-test-wrapping.md)
  for the wrapping rationale.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `BRIDGE_RIVETS_FEATURES_LIB_EXPECTED` | define | This macro is `#if`-usable. Its value is `__cpp_lib_expected`, or `0` when `__cpp_lib_expected` is not defined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_EXPECTED`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_FORMAT` | define | This macro is `#if`-usable. Its value is `__cpp_lib_format`, or `0` when `__cpp_lib_format` is not defined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_FORMAT`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_JTHREAD` | define | This macro is `#if`-usable. Its value is `__cpp_lib_jthread`, or `0` when `__cpp_lib_jthread` is not defined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_JTHREAD`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL` | define | This macro is `#if`-usable. Its value is `__cpp_lib_optional`, or `0` when `__cpp_lib_optional` is not defined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_PRINT` | define | This macro is `#if`-usable. Its value is `__cpp_lib_print`, or `0` when `__cpp_lib_print` is not defined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_PRINT`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_SPAN` | define | This macro is `#if`-usable. Its value is `__cpp_lib_span`, or `0` when `__cpp_lib_span` is not defined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_SPAN`](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN` | define | This macro is `#if`-usable. Its value is `__cpp_lib_stop_token`, or `0` when `__cpp_lib_stop_token` is not defined. | [`<version>::BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_expected` | variable | This value is `__cpp_lib_expected`, or `0` when `__cpp_lib_expected` is not defined. | [`<version>::lib_expected`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_format` | variable | This value is `__cpp_lib_format`, or `0` when `__cpp_lib_format` is not defined. | [`<version>::lib_format`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_jthread` | variable | This value is `__cpp_lib_jthread`, or `0` when `__cpp_lib_jthread` is not defined. | [`<version>::lib_jthread`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_optional` | variable | This value is `__cpp_lib_optional`, or `0` when `__cpp_lib_optional` is not defined. | [`<version>::lib_optional`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_print` | variable | This value is `__cpp_lib_print`, or `0` when `__cpp_lib_print` is not defined. | [`<version>::lib_print`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_span` | variable | This value is `__cpp_lib_span`, or `0` when `__cpp_lib_span` is not defined. | [`<version>::lib_span`](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_stop_token` | variable | This value is `__cpp_lib_stop_token`, or `0` when `__cpp_lib_stop_token` is not defined. | [`<version>::lib_stop_token`](https://en.cppreference.com/w/cpp/feature_test) |
<!-- BRIDGE-DOCS:END -->
