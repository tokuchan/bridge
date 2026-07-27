\page page_detectors Detectors

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/rivets/boost.hpp`
- `include/rivets/clang.hpp`
- `include/rivets/gcc.hpp`
- `include/rivets/libcxx.hpp`
- `include/rivets/libstdcxx.hpp`
- `include/rivets/msvc.hpp`
- `include/rivets/msvc_stl.hpp`
- `include/rivets/standard.hpp`
- `include/rivets/detail/detector.hpp`
<!-- BRIDGE-DOCS:END -->

TODO: narrative prose for this facility.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_CPLUSPLUS` | define | The active C++ standard, as a __cplusplus-style long integer. |
| `BRIDGE_CPP17` | define | The __cplusplus value for C++17. |
| `BRIDGE_CPP20` | define | The __cplusplus value for C++20. |
| `BRIDGE_CPP23` | define | The __cplusplus value for C++23. |
| `BRIDGE_HAS_CPP20` | define | Non-zero when compiling as C++20 or later. |
| `BRIDGE_HAS_CPP23` | define | Non-zero when compiling as C++23 or later. |
| `BRIDGE_RIVETS_CLANG_EQ` | define | \#if-usable form of bridge::rivets::clang::eq. |
| `BRIDGE_RIVETS_CLANG_GE` | define | \#if-usable form of bridge::rivets::clang::ge. |
| `BRIDGE_RIVETS_CLANG_GT` | define | \#if-usable form of bridge::rivets::clang::gt. |
| `BRIDGE_RIVETS_CLANG_LE` | define | \#if-usable form of bridge::rivets::clang::le. |
| `BRIDGE_RIVETS_CLANG_LT` | define | \#if-usable form of bridge::rivets::clang::lt. |
| `BRIDGE_RIVETS_CLANG_VERSION` | define | Clang's major version, or 0 when not compiling with Clang. |
| `BRIDGE_RIVETS_DEFINE_DETECTOR` | define | Generates a Named Detector: bridge::rivets::&lt;ns&gt;::&lt;cmp&gt;_&lt;n&gt;(). |
| `BRIDGE_RIVETS_DEFINE_DETECTOR_RANGE` | define | Generates a range Named Detector: bridge::rivets::&lt;ns&gt;::&lt;cmp1&gt;_&lt;n1&gt;_&lt;cmp2&gt;_&lt;n2&gt;(). |
| `BRIDGE_RIVETS_GCC_EQ` | define | \#if-usable form of bridge::rivets::gcc::eq. |
| `BRIDGE_RIVETS_GCC_GE` | define | \#if-usable form of bridge::rivets::gcc::ge. |
| `BRIDGE_RIVETS_GCC_GT` | define | \#if-usable form of bridge::rivets::gcc::gt. |
| `BRIDGE_RIVETS_GCC_LE` | define | \#if-usable form of bridge::rivets::gcc::le. |
| `BRIDGE_RIVETS_GCC_LT` | define | \#if-usable form of bridge::rivets::gcc::lt. |
| `BRIDGE_RIVETS_GCC_VERSION` | define | GCC's major version, or 0 when not compiling with real GCC. |
| `eq` | function | Is Clang's version exactly n? |
| `ge` | function | Is Clang's version greater than or equal to n? |
| `ge_18` | function | Named Detector: is clang's version ge 18? *. |
| `gt` | function | Is Clang's version greater than n? |
| `le` | function | Is Clang's version less than or equal to n? |
| `lt` | function | Is Clang's version less than n? |
| `version` | variable | Clang's major version, or 0 when not compiling with Clang. |
| `year_code_of` | function | Maps a short standard ordinal (17, 20, 23) to its __cplusplus-style year code (BRIDGE_CPP17 and friends). An ordinal bridge doesn't yet know maps to -1, which no real __cplusplus value can ever equal or exceed. |
<!-- BRIDGE-DOCS:END -->
