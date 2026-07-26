/// @file standard.hpp
/// @brief Language standard detection.
#pragma once

/// @def BRIDGE_CPLUSPLUS
/// @brief The active C++ standard, as a `__cplusplus`-style long integer.
///
/// MSVC only reports the correct `__cplusplus` value when built with
/// `/Zc:__cplusplus`; `_MSVC_LANG` always reflects the real target standard,
/// so it is preferred when available.
#if defined(_MSC_VER) && defined(_MSVC_LANG)
#    define BRIDGE_CPLUSPLUS _MSVC_LANG
#else
#    define BRIDGE_CPLUSPLUS __cplusplus
#endif

/// @def BRIDGE_CPP17
/// @brief The `__cplusplus` value for C++17.
#define BRIDGE_CPP17 201703L

/// @def BRIDGE_CPP20
/// @brief The `__cplusplus` value for C++20.
#define BRIDGE_CPP20 202002L

/// @def BRIDGE_CPP23
/// @brief The `__cplusplus` value for C++23.
#define BRIDGE_CPP23 202302L

#if BRIDGE_CPLUSPLUS < BRIDGE_CPP17
#    error "bridge requires at least C++17"
#endif

/// @def BRIDGE_HAS_CPP20
/// @brief Non-zero when compiling as C++20 or later.
#define BRIDGE_HAS_CPP20 (BRIDGE_CPLUSPLUS >= BRIDGE_CPP20)

/// @def BRIDGE_HAS_CPP23
/// @brief Non-zero when compiling as C++23 or later.
#define BRIDGE_HAS_CPP23 (BRIDGE_CPLUSPLUS >= BRIDGE_CPP23)

namespace bridge::rivets::standard {

/// @brief Maps a short standard ordinal (`17`, `20`, `23`) to its
///        `__cplusplus`-style year code (@ref BRIDGE_CPP17 and friends).
///        An ordinal bridge doesn't yet know maps to `-1`, which no real
///        `__cplusplus` value can ever equal or exceed.
/// @param ordinal The short form people actually say, e.g. `20` for C++20.
/// @return The matching `__cplusplus`-style year code, or `-1` if `ordinal`
///         isn't one bridge knows.
constexpr long year_code_of(int ordinal) {
    switch (ordinal) {
        case 17: return BRIDGE_CPP17;
        case 20: return BRIDGE_CPP20;
        case 23: return BRIDGE_CPP23;
        default: return -1;
    }
}

/// @brief Is the active C++ standard greater than `ordinal`?
/// @param ordinal The short form people actually say, e.g. `20` for C++20.
/// @return Whether the active `__cplusplus` value is greater than `ordinal`'s.
constexpr bool gt(int ordinal) { return BRIDGE_CPLUSPLUS > year_code_of(ordinal); }
/// @brief Is the active C++ standard greater than or equal to `ordinal`?
/// @param ordinal The short form people actually say, e.g. `20` for C++20.
/// @return Whether the active `__cplusplus` value is at least `ordinal`'s.
constexpr bool ge(int ordinal) { return BRIDGE_CPLUSPLUS >= year_code_of(ordinal); }
/// @brief Is the active C++ standard less than `ordinal`?
/// @param ordinal The short form people actually say, e.g. `20` for C++20.
/// @return Whether the active `__cplusplus` value is less than `ordinal`'s.
constexpr bool lt(int ordinal) { return BRIDGE_CPLUSPLUS < year_code_of(ordinal); }
/// @brief Is the active C++ standard less than or equal to `ordinal`?
/// @param ordinal The short form people actually say, e.g. `20` for C++20.
/// @return Whether the active `__cplusplus` value is at most `ordinal`'s.
constexpr bool le(int ordinal) { return BRIDGE_CPLUSPLUS <= year_code_of(ordinal); }
/// @brief Is the active C++ standard exactly `ordinal`?
/// @param ordinal The short form people actually say, e.g. `20` for C++20.
/// @return Whether the active `__cplusplus` value equals `ordinal`'s.
constexpr bool eq(int ordinal) { return BRIDGE_CPLUSPLUS == year_code_of(ordinal); }

} // namespace bridge::rivets::standard

// No BRIDGE_RIVETS_STANDARD_GT(n)-style #if macro family: unlike GCC/Clang,
// where the raw seed already is the number people compare against, the
// standard entity has two numbers (the short ordinal here vs. the
// __cplusplus-style year code) and mapping between them in the
// preprocessor would need every ordinal pre-registered as its own macro,
// silently misbehaving (rather than failing loudly) for one that isn't.
// BRIDGE_CPLUSPLUS/BRIDGE_CPP17/BRIDGE_CPP20/BRIDGE_CPP23 above are
// already the right #if-usable primitives for this entity.
