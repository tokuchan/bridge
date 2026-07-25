/// @file version.hpp
/// @brief Truss library version metadata.
#pragma once

#include <cstdint>

/// @def BRIDGE_TRUSS_VERSION_MAJOR
/// @brief Truss's major version number.
#define BRIDGE_TRUSS_VERSION_MAJOR 0

/// @def BRIDGE_TRUSS_VERSION_MINOR
/// @brief Truss's minor version number.
#define BRIDGE_TRUSS_VERSION_MINOR 1

/// @def BRIDGE_TRUSS_VERSION_PATCH
/// @brief Truss's patch version number.
#define BRIDGE_TRUSS_VERSION_PATCH 0

namespace bridge::detail::truss::cpp17::version {

/// @brief Truss's version, queryable at compile time or runtime.
struct version_info {
    /// @brief Major version number (see @ref BRIDGE_TRUSS_VERSION_MAJOR).
    std::uint32_t major = BRIDGE_TRUSS_VERSION_MAJOR;
    /// @brief Minor version number (see @ref BRIDGE_TRUSS_VERSION_MINOR).
    std::uint32_t minor = BRIDGE_TRUSS_VERSION_MINOR;
    /// @brief Patch version number (see @ref BRIDGE_TRUSS_VERSION_PATCH).
    std::uint32_t patch = BRIDGE_TRUSS_VERSION_PATCH;
};

/// @brief Symbols promoted to `bridge::exports::truss::version`.
namespace exports {
using bridge::detail::truss::cpp17::version::version_info;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::version

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
namespace bridge::exports::truss {
inline namespace cpp17 {
inline namespace version {
using namespace bridge::detail::truss::cpp17::version::exports;
} // namespace version
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::version_info;
} // namespace bridge::truss
