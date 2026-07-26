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

namespace bridge::rivets::features {

/// @brief Value of `__cpp_lib_optional`, or `0` if undefined.
inline constexpr long lib_optional = BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL;

/// @brief Value of `__cpp_lib_expected`, or `0` if undefined.
inline constexpr long lib_expected = BRIDGE_RIVETS_FEATURES_LIB_EXPECTED;

} // namespace bridge::rivets::features
