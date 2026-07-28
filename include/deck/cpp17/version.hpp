/// @file version.hpp
/// @brief This file holds Deck's version metadata.
#pragma once

#include <cstdint>

#include <truss/cpp17/version.hpp>

/// @def BRIDGE_DECK_VERSION_MAJOR
/// @brief This is Deck's major version number.
#define BRIDGE_DECK_VERSION_MAJOR 0

/// @def BRIDGE_DECK_VERSION_MINOR
/// @brief This is Deck's minor version number.
#define BRIDGE_DECK_VERSION_MINOR 1

/// @def BRIDGE_DECK_VERSION_PATCH
/// @brief This is Deck's patch version number.
#define BRIDGE_DECK_VERSION_PATCH 0

namespace bridge::detail::deck::cpp17::version {

/// @brief This struct holds Deck's version. It also holds the Truss
///        version Deck was compiled against.
struct version_info {
    /// @brief This is the major version number. See @ref
    ///        BRIDGE_DECK_VERSION_MAJOR.
    std::uint32_t major = BRIDGE_DECK_VERSION_MAJOR;
    /// @brief This is the minor version number. See @ref
    ///        BRIDGE_DECK_VERSION_MINOR.
    std::uint32_t minor = BRIDGE_DECK_VERSION_MINOR;
    /// @brief This is the patch version number. See @ref
    ///        BRIDGE_DECK_VERSION_PATCH.
    std::uint32_t patch = BRIDGE_DECK_VERSION_PATCH;
    /// @brief This is the version of the Truss library that Deck was
    ///        compiled against.
    bridge::truss::version_info truss{};
};

/// @brief This namespace promotes `version_info` to
///        `bridge::exports::deck::version`.
namespace exports {
using bridge::detail::deck::cpp17::version::version_info;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::version

/// @brief This is the Exports namespace for `version`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
namespace bridge::exports::deck {
inline namespace cpp17 {
inline namespace version {
using namespace bridge::detail::deck::cpp17::version::exports;
} // namespace version
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief This is Deck's public API.
namespace bridge::deck {
using bridge::exports::deck::version_info;
} // namespace bridge::deck
