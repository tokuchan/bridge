/// @file version.hpp
/// @brief Deck library version metadata.
#pragma once

#include <cstdint>

#include <truss/cpp17/version.hpp>

/// @def BRIDGE_DECK_VERSION_MAJOR
/// @brief Deck's major version number.
#define BRIDGE_DECK_VERSION_MAJOR 0

/// @def BRIDGE_DECK_VERSION_MINOR
/// @brief Deck's minor version number.
#define BRIDGE_DECK_VERSION_MINOR 1

/// @def BRIDGE_DECK_VERSION_PATCH
/// @brief Deck's patch version number.
#define BRIDGE_DECK_VERSION_PATCH 0

namespace bridge::detail::deck::cpp17::version {

/// @brief Deck's version, plus the Truss version it was built against.
struct version_info {
    /// @brief Major version number (see @ref BRIDGE_DECK_VERSION_MAJOR).
    std::uint32_t major = BRIDGE_DECK_VERSION_MAJOR;
    /// @brief Minor version number (see @ref BRIDGE_DECK_VERSION_MINOR).
    std::uint32_t minor = BRIDGE_DECK_VERSION_MINOR;
    /// @brief Patch version number (see @ref BRIDGE_DECK_VERSION_PATCH).
    std::uint32_t patch = BRIDGE_DECK_VERSION_PATCH;
    /// @brief Version of the Truss library Deck was compiled against.
    bridge::truss::version_info truss{};
};

/// @brief Symbols promoted to `bridge::exports::deck::version`.
namespace exports {
using bridge::detail::deck::cpp17::version::version_info;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::version

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
namespace bridge::exports::deck {
inline namespace cpp17 {
inline namespace version {
using namespace bridge::detail::deck::cpp17::version::exports;
} // namespace version
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::version_info;
} // namespace bridge::deck
