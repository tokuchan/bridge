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

A Detector answers "what version range is *this specific* compiler or
library at," as both a `constexpr bool` and an `#if`-usable macro.
**Standard** (`standard.hpp`), **GCC**, **Clang**, and **libstdc++**
are implemented today, each with the same shape: five generic
comparators (`gt`/`ge`/`lt`/`le`/`eq`) as Layer 1, plus however many
specific Named Detectors (`bridge::rivets::gcc::ge_13()`,
`BRIDGE_RIVETS_GCC_GE(13)`) as Layer 2 that this codebase or a
consumer actually needs -- generated via
`BRIDGE_RIVETS_DEFINE_DETECTOR`, never hand-written one at a time. See
\ref page_rivets for the shared mental model (the "sparse Cartesian
product" this two-layer split serves).

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

- **MSVC**, the **MSVC STL**, **libc++**, and **Boost** are registered
  facility headers but deliberately not yet implemented -- each file
  documents its own intended shape (and, for Boost specifically, why
  its Layer 1 needs a two-component `ge(major, minor)` form the other
  Entities don't) so the design is visible before the real work
  happens; that's why they don't appear in the symbol table below yet.
- **libstdc++** was implemented ahead of its own turn in this list,
  needed as a Detector-based override for a real libstdc++ Feature
  Test gap (`__cpp_lib_stop_token` is never defined, even though
  `stop_token` itself works) -- see docs/adr/0017.
- Detectors live directly in their own namespace with no
  Detail/Exports tiering (unlike Truss/Deck's facilities): there's
  never more than one right answer to "is GCC at least version 13," so
  there's nothing for a passthrough-or-polyfill selection to choose
  between.
- See [ADR-0006](https://github.com/tokuchan/bridge/blob/master/docs/adr/0006-detector-naming-calculus.md)
  for the naming calculus behind the Layer 1/Layer 2 split.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_CPLUSPLUS` | define | The active C++ standard, as a `__cplusplus`-style long integer. |
| `BRIDGE_CPP17` | define | The `__cplusplus` value for C++17. |
| `BRIDGE_CPP20` | define | The `__cplusplus` value for C++20. |
| `BRIDGE_CPP23` | define | The `__cplusplus` value for C++23. |
| `BRIDGE_HAS_CPP20` | define | Non-zero when compiling as C++20 or later. |
| `BRIDGE_HAS_CPP23` | define | Non-zero when compiling as C++23 or later. |
| `BRIDGE_RIVETS_CLANG_EQ` | define | `#if`-usable form of bridge::rivets::clang::eq. |
| `BRIDGE_RIVETS_CLANG_GE` | define | `#if`-usable form of bridge::rivets::clang::ge. |
| `BRIDGE_RIVETS_CLANG_GT` | define | `#if`-usable form of bridge::rivets::clang::gt. |
| `BRIDGE_RIVETS_CLANG_LE` | define | `#if`-usable form of bridge::rivets::clang::le. |
| `BRIDGE_RIVETS_CLANG_LT` | define | `#if`-usable form of bridge::rivets::clang::lt. |
| `BRIDGE_RIVETS_CLANG_VERSION` | define | Clang's major version, or `0` when not compiling with Clang. |
| `BRIDGE_RIVETS_DEFINE_DETECTOR` | define | Generates a Named Detector: `bridge::rivets::<ns>::<cmp>_<n>()`. |
| `BRIDGE_RIVETS_DEFINE_DETECTOR_RANGE` | define | Generates a range Named Detector: `bridge::rivets::<ns>::<cmp1>_<n1>_<cmp2>_<n2>()`. |
| `BRIDGE_RIVETS_GCC_EQ` | define | `#if`-usable form of bridge::rivets::gcc::eq. |
| `BRIDGE_RIVETS_GCC_GE` | define | `#if`-usable form of bridge::rivets::gcc::ge. |
| `BRIDGE_RIVETS_GCC_GT` | define | `#if`-usable form of bridge::rivets::gcc::gt. |
| `BRIDGE_RIVETS_GCC_LE` | define | `#if`-usable form of bridge::rivets::gcc::le. |
| `BRIDGE_RIVETS_GCC_LT` | define | `#if`-usable form of bridge::rivets::gcc::lt. |
| `BRIDGE_RIVETS_GCC_VERSION` | define | GCC's major version, or `0` when not compiling with real GCC. |
| `BRIDGE_RIVETS_LIBSTDCXX_EQ` | define | `#if`-usable form of bridge::rivets::libstdcxx::eq. |
| `BRIDGE_RIVETS_LIBSTDCXX_GE` | define | `#if`-usable form of bridge::rivets::libstdcxx::ge. |
| `BRIDGE_RIVETS_LIBSTDCXX_GT` | define | `#if`-usable form of bridge::rivets::libstdcxx::gt. |
| `BRIDGE_RIVETS_LIBSTDCXX_LE` | define | `#if`-usable form of bridge::rivets::libstdcxx::le. |
| `BRIDGE_RIVETS_LIBSTDCXX_LT` | define | `#if`-usable form of bridge::rivets::libstdcxx::lt. |
| `BRIDGE_RIVETS_LIBSTDCXX_VERSION` | define | libstdc++'s release version, or `0` when libstdc++ isn't the active standard library. |
| `bridge::rivets::clang::eq` | function | Is Clang's version exactly `n`? |
| `bridge::rivets::clang::ge` | function | Is Clang's version greater than or equal to `n`? |
| `bridge::rivets::clang::ge_18` | function | Named Detector: is clang's version ge 18? *. |
| `bridge::rivets::clang::gt` | function | Is Clang's version greater than `n`? |
| `bridge::rivets::clang::le` | function | Is Clang's version less than or equal to `n`? |
| `bridge::rivets::clang::lt` | function | Is Clang's version less than `n`? |
| `bridge::rivets::clang::version` | variable | Clang's major version, or `0` when not compiling with Clang. |
| `bridge::rivets::gcc::eq` | function | Is GCC's version exactly `n`? |
| `bridge::rivets::gcc::ge` | function | Is GCC's version greater than or equal to `n`? |
| `bridge::rivets::gcc::gt` | function | Is GCC's version greater than `n`? |
| `bridge::rivets::gcc::le` | function | Is GCC's version less than or equal to `n`? |
| `bridge::rivets::gcc::lt` | function | Is GCC's version less than `n`? |
| `bridge::rivets::gcc::version` | variable | GCC's major version, or `0` when not compiling with real GCC. |
| `bridge::rivets::libstdcxx::eq` | function | Is libstdc++'s version exactly `n`? |
| `bridge::rivets::libstdcxx::ge` | function | Is libstdc++'s version greater than or equal to `n`? |
| `bridge::rivets::libstdcxx::gt` | function | Is libstdc++'s version greater than `n`? |
| `bridge::rivets::libstdcxx::le` | function | Is libstdc++'s version less than or equal to `n`? |
| `bridge::rivets::libstdcxx::lt` | function | Is libstdc++'s version less than `n`? |
| `bridge::rivets::libstdcxx::version` | variable | libstdc++'s release version, or `0` when it isn't the active standard library. |
| `bridge::rivets::standard::eq` | function | Is the active C++ standard exactly `ordinal`? |
| `bridge::rivets::standard::ge` | function | Is the active C++ standard greater than or equal to `ordinal`? |
| `bridge::rivets::standard::gt` | function | Is the active C++ standard greater than `ordinal`? |
| `bridge::rivets::standard::le` | function | Is the active C++ standard less than or equal to `ordinal`? |
| `bridge::rivets::standard::lt` | function | Is the active C++ standard less than `ordinal`? |
| `bridge::rivets::standard::year_code_of` | function | Maps a short standard ordinal (`17`, `20`, `23`) to its `__cplusplus`-style year code (BRIDGE_CPP17 and friends). An ordinal bridge doesn't yet know maps to `-1`, which no real `__cplusplus` value can ever equal or exceed. |
<!-- BRIDGE-DOCS:END -->
