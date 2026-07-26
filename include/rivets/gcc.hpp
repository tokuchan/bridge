/// @file gcc.hpp
/// @brief GCC compiler version Detector. See docs/adr/0006-detector-
///        naming-calculus.md and include/rivets/detail/detector.hpp for
///        the calculus this follows.
#pragma once

#include <rivets/detail/detector.hpp>

/// @def BRIDGE_RIVETS_GCC_VERSION
/// @brief GCC's major version, or `0` when not compiling with real GCC.
///
/// Clang also defines `__GNUC__`/`__GNUC_MINOR__` for source
/// compatibility, so real-GCC detection excludes `__clang__` explicitly.
#if defined(__GNUC__) && !defined(__clang__)
#    define BRIDGE_RIVETS_GCC_VERSION __GNUC__
#else
#    define BRIDGE_RIVETS_GCC_VERSION 0
#endif

namespace bridge::rivets::gcc {

/// @brief GCC's major version, or `0` when not compiling with real GCC.
inline constexpr int version = BRIDGE_RIVETS_GCC_VERSION;

/// @brief Is GCC's version greater than `n`?
/// @param n The version number to compare against.
/// @return Whether GCC's major version is greater than `n`.
constexpr bool gt(int n) { return version > n; }
/// @brief Is GCC's version greater than or equal to `n`?
/// @param n The version number to compare against.
/// @return Whether GCC's major version is greater than or equal to `n`.
constexpr bool ge(int n) { return version >= n; }
/// @brief Is GCC's version less than `n`?
/// @param n The version number to compare against.
/// @return Whether GCC's major version is less than `n`.
constexpr bool lt(int n) { return version < n; }
/// @brief Is GCC's version less than or equal to `n`?
/// @param n The version number to compare against.
/// @return Whether GCC's major version is less than or equal to `n`.
constexpr bool le(int n) { return version <= n; }
/// @brief Is GCC's version exactly `n`?
/// @param n The version number to compare against.
/// @return Whether GCC's major version equals `n`.
constexpr bool eq(int n) { return version == n; }

} // namespace bridge::rivets::gcc

/// @def BRIDGE_RIVETS_GCC_GT
/// @brief `#if`-usable form of bridge::rivets::gcc::gt.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_GCC_GT(n) (BRIDGE_RIVETS_GCC_VERSION > (n))
/// @def BRIDGE_RIVETS_GCC_GE
/// @brief `#if`-usable form of bridge::rivets::gcc::ge.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_GCC_GE(n) (BRIDGE_RIVETS_GCC_VERSION >= (n))
/// @def BRIDGE_RIVETS_GCC_LT
/// @brief `#if`-usable form of bridge::rivets::gcc::lt.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_GCC_LT(n) (BRIDGE_RIVETS_GCC_VERSION < (n))
/// @def BRIDGE_RIVETS_GCC_LE
/// @brief `#if`-usable form of bridge::rivets::gcc::le.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_GCC_LE(n) (BRIDGE_RIVETS_GCC_VERSION <= (n))
/// @def BRIDGE_RIVETS_GCC_EQ
/// @brief `#if`-usable form of bridge::rivets::gcc::eq.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_GCC_EQ(n) (BRIDGE_RIVETS_GCC_VERSION == (n))

// Named Detector, proving Layer 2 end to end: GCC 13 is the oldest
// compiler live in this project's own matrix (docs/adr/0004).
BRIDGE_RIVETS_DEFINE_DETECTOR(gcc, ge, 13)
