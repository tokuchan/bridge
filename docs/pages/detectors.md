\page page_detectors Detectors

<!-- BRIDGE-DOCS:BEGIN header-link -->
<!-- BRIDGE-DOCS:END -->

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

## Synopsis

A Detector answers one question: what version range is this
specific compiler or library at? A Detector gives this answer as
both a `constexpr bool` and an `#if`-usable macro.

Standard (`standard.hpp`), GCC, Clang, and libstdc++ are implemented
today. Each has the same shape. Layer 1 is five generic comparators:
`gt`, `ge`, `lt`, `le`, and `eq`. Layer 2 is however many specific
Named Detectors this codebase or a consumer actually needs, for
example `bridge::rivets::gcc::ge_13()` and `BRIDGE_RIVETS_GCC_GE(13)`.
`BRIDGE_RIVETS_DEFINE_DETECTOR` generates every Layer 2 Named
Detector. Nobody hand-writes a Named Detector one at a time.

See \ref page_rivets for the shared mental model: the "sparse
Cartesian product" this two-layer split serves.

## Example

```cpp
#include <rivets/gcc.hpp>

int classify() {
    if constexpr (bridge::rivets::gcc::ge(13)) {
        return 1; // GCC 13+: known-good behavior
    } else {
        return 0; // Older GCC, or a different compiler entirely (version == 0)
    }
}

#if BRIDGE_RIVETS_GCC_GE(13)
constexpr bool has_workaround = false;
#else
constexpr bool has_workaround = true;
#endif

int main() {
    classify();
    (void)has_workaround;
}
```

## Notes

- MSVC, the MSVC STL, libc++, and Boost are registered facility
  headers. This facility deliberately has not implemented them yet.
  Each file documents its own intended shape. For Boost specifically,
  its file also documents why Boost's Layer 1 needs a two-component
  `ge(major, minor)` form the other Entities don't. This design is
  visible before the real work happens. This is why these four
  Entities do not appear in the symbol table below yet.
- This facility implemented libstdc++ ahead of its own turn in this
  list. libstdc++ needed a Detector-based override, for a real
  libstdc++ Feature Test gap: `__cpp_lib_stop_token` is never
  defined, even though `stop_token` itself works. See docs/adr/0017.
- Detectors live directly in their own namespace, with no
  Detail/Exports tiering, unlike Truss's and Deck's facilities.
  There is never more than one right answer to "is GCC at least
  version 13?" So there is nothing for a passthrough-or-polyfill
  choice to choose between.
- See [ADR-0006](https://github.com/tokuchan/bridge/blob/master/docs/adr/0006-detector-naming-calculus.md)
  for the naming calculus behind the Layer 1/Layer 2 split.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_CPLUSPLUS` | define | This is the active C++ standard, as a `__cplusplus`-style long integer. |
| `BRIDGE_CPP17` | define | This is the `__cplusplus` value for C++17. |
| `BRIDGE_CPP20` | define | This is the `__cplusplus` value for C++20. |
| `BRIDGE_CPP23` | define | This is the `__cplusplus` value for C++23. |
| `BRIDGE_HAS_CPP20` | define | This is non-zero when compiling as C++20 or later. |
| `BRIDGE_HAS_CPP23` | define | This is non-zero when compiling as C++23 or later. |
| `BRIDGE_RIVETS_CLANG_EQ` | define | This is the `#if`-usable form of `bridge::rivets::clang::eq`. |
| `BRIDGE_RIVETS_CLANG_GE` | define | This is the `#if`-usable form of `bridge::rivets::clang::ge`. |
| `BRIDGE_RIVETS_CLANG_GT` | define | This is the `#if`-usable form of `bridge::rivets::clang::gt`. |
| `BRIDGE_RIVETS_CLANG_LE` | define | This is the `#if`-usable form of `bridge::rivets::clang::le`. |
| `BRIDGE_RIVETS_CLANG_LT` | define | This is the `#if`-usable form of `bridge::rivets::clang::lt`. |
| `BRIDGE_RIVETS_CLANG_VERSION` | define | This is Clang's major version, or `0` when not compiling with Clang. |
| `BRIDGE_RIVETS_DEFINE_DETECTOR` | define | This macro generates a Named Detector: `bridge::rivets::<ns>::<cmp>_<n>()`. |
| `BRIDGE_RIVETS_DEFINE_DETECTOR_RANGE` | define | This macro generates a range Named Detector: `bridge::rivets::<ns>::<cmp1>_<n1>_<cmp2>_<n2>()`. |
| `BRIDGE_RIVETS_GCC_EQ` | define | This is the `#if`-usable form of `bridge::rivets::gcc::eq`. |
| `BRIDGE_RIVETS_GCC_GE` | define | This is the `#if`-usable form of `bridge::rivets::gcc::ge`. |
| `BRIDGE_RIVETS_GCC_GT` | define | This is the `#if`-usable form of `bridge::rivets::gcc::gt`. |
| `BRIDGE_RIVETS_GCC_LE` | define | This is the `#if`-usable form of `bridge::rivets::gcc::le`. |
| `BRIDGE_RIVETS_GCC_LT` | define | This is the `#if`-usable form of `bridge::rivets::gcc::lt`. |
| `BRIDGE_RIVETS_GCC_VERSION` | define | This is GCC's major version, or `0` when not compiling with real GCC. |
| `BRIDGE_RIVETS_LIBSTDCXX_EQ` | define | This is the `#if`-usable form of `bridge::rivets::libstdcxx::eq`. |
| `BRIDGE_RIVETS_LIBSTDCXX_GE` | define | This is the `#if`-usable form of `bridge::rivets::libstdcxx::ge`. |
| `BRIDGE_RIVETS_LIBSTDCXX_GT` | define | This is the `#if`-usable form of `bridge::rivets::libstdcxx::gt`. |
| `BRIDGE_RIVETS_LIBSTDCXX_LE` | define | This is the `#if`-usable form of `bridge::rivets::libstdcxx::le`. |
| `BRIDGE_RIVETS_LIBSTDCXX_LT` | define | This is the `#if`-usable form of `bridge::rivets::libstdcxx::lt`. |
| `BRIDGE_RIVETS_LIBSTDCXX_VERSION` | define | This is libstdc++'s release version, or `0` when libstdc++ is not the active standard library. |
| `bridge::rivets::clang::eq` | function | This checks whether Clang's version is exactly `n`. |
| `bridge::rivets::clang::ge` | function | This checks whether Clang's version is greater than or equal to `n`. |
| `bridge::rivets::clang::ge_18` | function | This is a Named Detector. This checks whether clang's version ge 18. *. |
| `bridge::rivets::clang::gt` | function | This checks whether Clang's version is greater than `n`. |
| `bridge::rivets::clang::le` | function | This checks whether Clang's version is less than or equal to `n`. |
| `bridge::rivets::clang::lt` | function | This checks whether Clang's version is less than `n`. |
| `bridge::rivets::clang::version` | variable | This is Clang's major version, or `0` when not compiling with Clang. |
| `bridge::rivets::gcc::eq` | function | This checks whether GCC's version is exactly `n`. |
| `bridge::rivets::gcc::ge` | function | This checks whether GCC's version is greater than or equal to `n`. |
| `bridge::rivets::gcc::gt` | function | This checks whether GCC's version is greater than `n`. |
| `bridge::rivets::gcc::le` | function | This checks whether GCC's version is less than or equal to `n`. |
| `bridge::rivets::gcc::lt` | function | This checks whether GCC's version is less than `n`. |
| `bridge::rivets::gcc::version` | variable | This is GCC's major version, or `0` when not compiling with real GCC. |
| `bridge::rivets::libstdcxx::eq` | function | This checks whether libstdc++'s version is exactly `n`. |
| `bridge::rivets::libstdcxx::ge` | function | This checks whether libstdc++'s version is greater than or equal to `n`. |
| `bridge::rivets::libstdcxx::gt` | function | This checks whether libstdc++'s version is greater than `n`. |
| `bridge::rivets::libstdcxx::le` | function | This checks whether libstdc++'s version is less than or equal to `n`. |
| `bridge::rivets::libstdcxx::lt` | function | This checks whether libstdc++'s version is less than `n`. |
| `bridge::rivets::libstdcxx::version` | variable | This is libstdc++'s release version, or `0` when it is not the active standard library. |
| `bridge::rivets::standard::eq` | function | This checks whether the active C++ standard is exactly `ordinal`. |
| `bridge::rivets::standard::ge` | function | This checks whether the active C++ standard is greater than or equal to `ordinal`. |
| `bridge::rivets::standard::gt` | function | This checks whether the active C++ standard is greater than `ordinal`. |
| `bridge::rivets::standard::le` | function | This checks whether the active C++ standard is less than or equal to `ordinal`. |
| `bridge::rivets::standard::lt` | function | This checks whether the active C++ standard is less than `ordinal`. |
| `bridge::rivets::standard::year_code_of` | function | This function maps a short standard ordinal (`17`, `20`, `23`) to its `__cplusplus`-style year code (BRIDGE_CPP17 and friends). An ordinal bridge does not yet know maps to `-1`. No real `__cplusplus` value can ever equal or exceed `-1`. |
<!-- BRIDGE-DOCS:END -->
