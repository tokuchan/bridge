/// @file clang.hpp
/// @brief This file holds the Clang compiler version Detector. See
///        docs/adr/0006-detector-naming-calculus.md and
///        include/rivets/detail/detector.hpp for the calculus this
///        file follows.
#pragma once

#include <rivets/detail/detector.hpp>

/// @def BRIDGE_RIVETS_CLANG_VERSION
/// @brief This is Clang's major version, or `0` when not
///        compiling with Clang.
#if defined(__clang__)
#    define BRIDGE_RIVETS_CLANG_VERSION __clang_major__
#else
#    define BRIDGE_RIVETS_CLANG_VERSION 0
#endif

namespace bridge::rivets::clang {

/// @brief This is Clang's major version, or `0` when not
///        compiling with Clang.
inline constexpr int version = BRIDGE_RIVETS_CLANG_VERSION;

/// @brief This checks whether Clang's version is greater than `n`.
/// @param n The version number to compare against.
/// @return Whether Clang's major version is greater than `n`.
constexpr bool gt(int n) { return version > n; }
/// @brief This checks whether Clang's version is greater than or
///        equal to `n`.
/// @param n The version number to compare against.
/// @return Whether Clang's major version is greater than or equal to `n`.
constexpr bool ge(int n) { return version >= n; }
/// @brief This checks whether Clang's version is less than `n`.
/// @param n The version number to compare against.
/// @return Whether Clang's major version is less than `n`.
constexpr bool lt(int n) { return version < n; }
/// @brief This checks whether Clang's version is less than or
///        equal to `n`.
/// @param n The version number to compare against.
/// @return Whether Clang's major version is less than or equal to `n`.
constexpr bool le(int n) { return version <= n; }
/// @brief This checks whether Clang's version is exactly `n`.
/// @param n The version number to compare against.
/// @return Whether Clang's major version equals `n`.
constexpr bool eq(int n) { return version == n; }

} // namespace bridge::rivets::clang

/// @def BRIDGE_RIVETS_CLANG_GT
/// @brief This is the `#if`-usable form of `bridge::rivets::clang::gt`.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_CLANG_GT(n) (BRIDGE_RIVETS_CLANG_VERSION > (n))
/// @def BRIDGE_RIVETS_CLANG_GE
/// @brief This is the `#if`-usable form of `bridge::rivets::clang::ge`.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_CLANG_GE(n) (BRIDGE_RIVETS_CLANG_VERSION >= (n))
/// @def BRIDGE_RIVETS_CLANG_LT
/// @brief This is the `#if`-usable form of `bridge::rivets::clang::lt`.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_CLANG_LT(n) (BRIDGE_RIVETS_CLANG_VERSION < (n))
/// @def BRIDGE_RIVETS_CLANG_LE
/// @brief This is the `#if`-usable form of `bridge::rivets::clang::le`.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_CLANG_LE(n) (BRIDGE_RIVETS_CLANG_VERSION <= (n))
/// @def BRIDGE_RIVETS_CLANG_EQ
/// @brief This is the `#if`-usable form of `bridge::rivets::clang::eq`.
/// @param n The version number to compare against.
#define BRIDGE_RIVETS_CLANG_EQ(n) (BRIDGE_RIVETS_CLANG_VERSION == (n))

// Named Detector, proving Layer 2 end to end: Clang 18 is the oldest
// compiler live in this project's own matrix (docs/adr/0004).
BRIDGE_RIVETS_DEFINE_DETECTOR(clang, ge, 18)
