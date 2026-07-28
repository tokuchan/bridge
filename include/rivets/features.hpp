/// @file features.hpp
/// @brief This file holds Feature Test wrapping. See
///        docs/adr/0007-feature-test-wrapping.md for the full
///        rationale.
///
/// A Feature Test is different from a Detector
/// (include/rivets/detail/detector.hpp). A Feature Test wraps one
/// SD-6 feature-test macro directly. The compiler or the standard
/// library already publishes this macro. A Feature Test does not
/// compute a range over a seeded version, the way a Detector does.
///
/// Each wrap is hand-written: one `#ifdef` block per feature.
/// Detectors are hand-written for the same reason ([cpp.rescan]): a
/// macro invocation can never emit a `#define`/`#ifdef` on another
/// macro's behalf. This is why there is no generator here.
///
/// To add a new Feature Test, copy this file's pattern:
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
/// Rivets does not interpret what a Feature Test's value means, for
/// any particular consumer. For example, Rivets does not know that
/// `202110L` is the threshold `optional`'s monadic methods need. That
/// is domain knowledge for whatever consumes the value, Truss or
/// Deck. It is not a detection concern.
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
/// @brief This macro is `#if`-usable. Its value is
///        `__cpp_lib_optional`, or `0` when `__cpp_lib_optional` is
///        not defined.
#ifdef __cpp_lib_optional
#    define BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL __cpp_lib_optional
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_EXPECTED
/// @brief This macro is `#if`-usable. Its value is
///        `__cpp_lib_expected`, or `0` when `__cpp_lib_expected` is
///        not defined.
#ifdef __cpp_lib_expected
#    define BRIDGE_RIVETS_FEATURES_LIB_EXPECTED __cpp_lib_expected
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_EXPECTED 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_FORMAT
/// @brief This macro is `#if`-usable. Its value is
///        `__cpp_lib_format`, or `0` when `__cpp_lib_format` is not
///        defined.
#ifdef __cpp_lib_format
#    define BRIDGE_RIVETS_FEATURES_LIB_FORMAT __cpp_lib_format
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_FORMAT 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_PRINT
/// @brief This macro is `#if`-usable. Its value is `__cpp_lib_print`,
///        or `0` when `__cpp_lib_print` is not defined.
#ifdef __cpp_lib_print
#    define BRIDGE_RIVETS_FEATURES_LIB_PRINT __cpp_lib_print
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_PRINT 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_SPAN
/// @brief This macro is `#if`-usable. Its value is `__cpp_lib_span`,
///        or `0` when `__cpp_lib_span` is not defined.
#ifdef __cpp_lib_span
#    define BRIDGE_RIVETS_FEATURES_LIB_SPAN __cpp_lib_span
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_SPAN 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_JTHREAD
/// @brief This macro is `#if`-usable. Its value is
///        `__cpp_lib_jthread`, or `0` when `__cpp_lib_jthread` is not
///        defined.
#ifdef __cpp_lib_jthread
#    define BRIDGE_RIVETS_FEATURES_LIB_JTHREAD __cpp_lib_jthread
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_JTHREAD 0
#endif

/// @def BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN
/// @brief This macro is `#if`-usable. Its value is
///        `__cpp_lib_stop_token`, or `0` when `__cpp_lib_stop_token`
///        is not defined.
///
///        This macro's value is always `0` on every GCC (13-15) and
///        Clang (20) this project's matrix covers. libstdc++ never
///        defines `__cpp_lib_stop_token`, even though `stop_token`
///        itself works. See `bridge::rivets::libstdcxx` for the
///        Detector-based override this drives in
///        `deck/cpp17/jthread.hpp`.
#ifdef __cpp_lib_stop_token
#    define BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN __cpp_lib_stop_token
#else
#    define BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN 0
#endif

namespace bridge::rivets::features {

/// @brief This value is `__cpp_lib_optional`, or `0` when
///        `__cpp_lib_optional` is not defined.
inline constexpr long lib_optional = BRIDGE_RIVETS_FEATURES_LIB_OPTIONAL;

/// @brief This value is `__cpp_lib_expected`, or `0` when
///        `__cpp_lib_expected` is not defined.
inline constexpr long lib_expected = BRIDGE_RIVETS_FEATURES_LIB_EXPECTED;

/// @brief This value is `__cpp_lib_format`, or `0` when `__cpp_lib_format`
///        is not defined.
inline constexpr long lib_format = BRIDGE_RIVETS_FEATURES_LIB_FORMAT;

/// @brief This value is `__cpp_lib_print`, or `0` when `__cpp_lib_print`
///        is not defined.
inline constexpr long lib_print = BRIDGE_RIVETS_FEATURES_LIB_PRINT;

/// @brief This value is `__cpp_lib_span`, or `0` when `__cpp_lib_span`
///        is not defined.
inline constexpr long lib_span = BRIDGE_RIVETS_FEATURES_LIB_SPAN;

/// @brief This value is `__cpp_lib_jthread`, or `0` when `__cpp_lib_jthread`
///        is not defined.
inline constexpr long lib_jthread = BRIDGE_RIVETS_FEATURES_LIB_JTHREAD;

/// @brief This value is `__cpp_lib_stop_token`, or `0` when
///        `__cpp_lib_stop_token` is not defined.
inline constexpr long lib_stop_token = BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN;

} // namespace bridge::rivets::features
