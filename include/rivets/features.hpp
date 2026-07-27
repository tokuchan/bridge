/// @file features.hpp
/// @brief Feature Test wrapping. See docs/adr/0007-feature-test-
///        wrapping.md for the full rationale.
///
/// A Feature Test is different in kind from a Detector
/// (include/rivets/detail/detector.hpp): it wraps an SD-6 feature-test
/// macro that the compiler/stdlib already publishes directly, rather
/// than computing a range over a seeded version. Each wrap is
/// hand-written, one `#ifdef` block per feature — for the same
/// [cpp.rescan] reason Detectors are hand-written, a macro invocation
/// can never emit a `#define`/`#ifdef` on another macro's behalf, so
/// there is no generator here, by design. To add a new Feature Test,
/// copy this file's pattern:
///
/// ```cpp
/// #ifdef __cpp_some_feature
/// #    define BRIDGE_RIVETS_FEATURES_SOME_FEATURE __cpp_some_feature
/// #else
/// #    define BRIDGE_RIVETS_FEATURES_SOME_FEATURE 0
/// #endif
///
/// namespace bridge::rivets::features {
/// inline constexpr long some_feature = BRIDGE_RIVETS_FEATURES_SOME_FEATURE;
/// }
/// ```
///
/// Rivets does not interpret what a Feature Test's value *means* for any
/// particular consumer (e.g. that `202110L` is the threshold `optional`'s
/// monadic methods need) — that's domain knowledge for whatever consumes
/// it (Truss/Deck), not a detection concern.
#pragma once

// Library feature-test macros (__cpp_lib_*) are only guaranteed visible
// once the header that defines the corresponding feature has actually
// been included -- <version> is the SD-6-designed escape hatch: it
// exposes every one of them without pulling in the full library
// implementation behind each. Without this, a consumer that checks
// BRIDGE_RIVETS_FEATURES_LIB_EXPECTED *before* deciding whether it's
// even safe to #include <expected> (since that header doesn't exist at
// all pre-C++23) would see a false 0 on every ecosystem, regardless of
// real support -- confirmed by hitting exactly that failure mode in
// deck/cpp17/expected.hpp before adding this, not assumed. Verified
// <version> itself is safely includable under this project's C++17
// floor on both GCC and Clang before relying on it.
#include <version>

/// @def BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL
/// @brief `#if`-usable value of `__cpp_lib_optional`, or `0` if undefined.
#ifdef __cpp_lib_optional
#    define BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL __cpp_lib_optional
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_EXPECTED
/// @brief `#if`-usable value of `__cpp_lib_expected`, or `0` if undefined.
#ifdef __cpp_lib_expected
#    define BRIDGE_RIVETS_FEATURES_LIB_EXPECTED __cpp_lib_expected
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_EXPECTED 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_FORMAT
/// @brief `#if`-usable value of `__cpp_lib_format`, or `0` if undefined.
#ifdef __cpp_lib_format
#    define BRIDGE_RIVETS_FEATURES_LIB_FORMAT __cpp_lib_format
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_FORMAT 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_PRINT
/// @brief `#if`-usable value of `__cpp_lib_print`, or `0` if undefined.
#ifdef __cpp_lib_print
#    define BRIDGE_RIVETS_FEATURES_LIB_PRINT __cpp_lib_print
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_PRINT 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_SPAN
/// @brief `#if`-usable value of `__cpp_lib_span`, or `0` if undefined.
#ifdef __cpp_lib_span
#    define BRIDGE_RIVETS_FEATURES_LIB_SPAN __cpp_lib_span
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_SPAN 0
#endif

namespace bridge::rivets::features {

/// @brief Value of `__cpp_lib_optional`, or `0` if undefined.
inline constexpr long lib_optional = BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL;

/// @brief Value of `__cpp_lib_expected`, or `0` if undefined.
inline constexpr long lib_expected = BRIDGE_RIVETS_FEATURES_LIB_EXPECTED;

/// @brief Value of `__cpp_lib_format`, or `0` if undefined.
inline constexpr long lib_format = BRIDGE_RIVETS_FEATURES_LIB_FORMAT;

/// @brief Value of `__cpp_lib_print`, or `0` if undefined.
inline constexpr long lib_print = BRIDGE_RIVETS_FEATURES_LIB_PRINT;

/// @brief Value of `__cpp_lib_span`, or `0` if undefined.
inline constexpr long lib_span = BRIDGE_RIVETS_FEATURES_LIB_SPAN;

} // namespace bridge::rivets::features
