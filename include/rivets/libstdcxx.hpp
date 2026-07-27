/// @file libstdcxx.hpp
/// @brief libstdc++ (GNU) STL implementation version Detector. See
///        docs/adr/0006-detector-naming-calculus.md and
///        include/rivets/detail/detector.hpp for the calculus this
///        follows.
#pragma once

#include <version>

#include <rivets/detail/detector.hpp>

/// @def BRIDGE_RIVETS_LIBSTDCXX_VERSION
/// @brief libstdc++'s release version, or `0` when libstdc++ isn't the
///        active standard library.
///
/// `_GLIBCXX_RELEASE` (added in GCC 7, equal to the GCC major version
/// that shipped that libstdc++ release) is simpler than the older
/// `__GLIBCXX__` datestamp macro, which should not be used for new
/// code. `__GLIBCXX__`'s presence indicates libstdc++ is the active
/// standard library at all, which — per the Entity list in
/// `CONTEXT.md` — is independent of which *compiler* is in use: Clang
/// can be paired with either libstdc++ or libc++, so this must not
/// assume libstdc++ just because `bridge::rivets::gcc::version` is
/// nonzero. Confirmed via probe that `<version>` alone is sufficient
/// to make both macros visible, without needing any other libstdc++
/// header included first.
#if defined(__GLIBCXX__)
#    define BRIDGE_RIVETS_LIBSTDCXX_VERSION _GLIBCXX_RELEASE
#else
#    define BRIDGE_RIVETS_LIBSTDCXX_VERSION 0
#endif

namespace bridge::rivets::libstdcxx {

/// @brief libstdc++'s release version, or `0` when it isn't the active
///        standard library.
inline constexpr int version = BRIDGE_RIVETS_LIBSTDCXX_VERSION;

/// @brief Is libstdc++'s version greater than `n`?
/// @param n The version number to compare against.
/// @return Whether libstdc++'s release version is greater than `n`.
constexpr bool gt(int n) { return version > n; }
/// @brief Is libstdc++'s version greater than or equal to `n`?
/// @param n The version number to compare against.
/// @return Whether libstdc++'s release version is greater than or equal to `n`.
constexpr bool ge(int n) { return version >= n; }
/// @brief Is libstdc++'s version less than `n`?
/// @param n The version number to compare against.
/// @return Whether libstdc++'s release version is less than `n`.
constexpr bool lt(int n) { return version < n; }
/// @brief Is libstdc++'s version less than or equal to `n`?
/// @param n The version number to compare against.
/// @return Whether libstdc++'s release version is less than or equal to `n`.
constexpr bool le(int n) { return version <= n; }
/// @brief Is libstdc++'s version exactly `n`?
/// @param n The version number to compare against.
/// @return Whether libstdc++'s release version equals `n`.
constexpr bool eq(int n) { return version == n; }

} // namespace bridge::rivets::libstdcxx

/// @def BRIDGE_RIVETS_LIBSTDCXX_GT
/// @brief `#if`-usable form of bridge::rivets::libstdcxx::gt.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_LIBSTDCXX_GT(n) (BRIDGE_RIVETS_LIBSTDCXX_VERSION > (n))
/// @def BRIDGE_RIVETS_LIBSTDCXX_GE
/// @brief `#if`-usable form of bridge::rivets::libstdcxx::ge.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_LIBSTDCXX_GE(n) (BRIDGE_RIVETS_LIBSTDCXX_VERSION >= (n))
/// @def BRIDGE_RIVETS_LIBSTDCXX_LT
/// @brief `#if`-usable form of bridge::rivets::libstdcxx::lt.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_LIBSTDCXX_LT(n) (BRIDGE_RIVETS_LIBSTDCXX_VERSION < (n))
/// @def BRIDGE_RIVETS_LIBSTDCXX_LE
/// @brief `#if`-usable form of bridge::rivets::libstdcxx::le.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_LIBSTDCXX_LE(n) (BRIDGE_RIVETS_LIBSTDCXX_VERSION <= (n))
/// @def BRIDGE_RIVETS_LIBSTDCXX_EQ
/// @brief `#if`-usable form of bridge::rivets::libstdcxx::eq.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_LIBSTDCXX_EQ(n) (BRIDGE_RIVETS_LIBSTDCXX_VERSION == (n))

// Named Detector, proving Layer 2 end to end: libstdc++ 13 (paired
// with GCC 13) is the oldest release live in this project's own
// matrix (docs/adr/0004).
BRIDGE_RIVETS_DEFINE_DETECTOR(libstdcxx, ge, 13)
