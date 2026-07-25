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
