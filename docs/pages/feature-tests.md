\page page_feature_tests Feature Tests

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/rivets/features.hpp`
<!-- BRIDGE-DOCS:END -->

See \ref page_rivets for how Feature Tests relate to Detectors --
in short, a Feature Test wraps a single SD-6 feature-test macro (e.g.
`__cpp_lib_optional`) directly, in both a `constexpr` form and an
`#if`-usable macro form, and is the preferred signal for "does this
specific ecosystem support this specific library feature." Unlike a
Detector, there's no comparator calculus here: the macro's value (or
`0` if the ecosystem doesn't define it at all -- confirmed to need
`<version>` included first, or the macro may be invisible even on an
ecosystem that does support the feature) already *is* the fact, not a
range Rivets computes. Every Truss/Deck facility that offers real
`std::` passthrough (`optional`, `expected`, `format`, `print`) is
gated by exactly one of these. See
[ADR-0007](https://github.com/tokuchan/bridge/blob/master/docs/adr/0007-feature-test-wrapping.md)
for the wrapping rationale.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `BRIDGE_RIVETS_FEATURES_LIB_EXPECTED` | define | \#if-usable value of __cpp_lib_expected, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_FORMAT` | define | \#if-usable value of __cpp_lib_format, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL` | define | \#if-usable value of __cpp_lib_optional, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_PRINT` | define | \#if-usable value of __cpp_lib_print, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `BRIDGE_RIVETS_FEATURES_LIB_SPAN` | define | \#if-usable value of __cpp_lib_span, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_expected` | variable | Value of __cpp_lib_expected, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_format` | variable | Value of __cpp_lib_format, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_optional` | variable | Value of __cpp_lib_optional, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_print` | variable | Value of __cpp_lib_print, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
| `bridge::rivets::features::lib_span` | variable | Value of __cpp_lib_span, or 0 if undefined. | [https://en.cppreference.com/w/cpp/feature_test](https://en.cppreference.com/w/cpp/feature_test) |
<!-- BRIDGE-DOCS:END -->
