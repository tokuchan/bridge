/// @file version.hpp
/// @brief This file holds Truss's version metadata.
#pragma once

#include <cstdint>

/// @def BRIDGE_TRUSS_VERSION_MAJOR
/// @brief This is Truss's major version number.
#define BRIDGE_TRUSS_VERSION_MAJOR 0

/// @def BRIDGE_TRUSS_VERSION_MINOR
/// @brief This is Truss's minor version number.
#define BRIDGE_TRUSS_VERSION_MINOR 1

/// @def BRIDGE_TRUSS_VERSION_PATCH
/// @brief This is Truss's patch version number.
#define BRIDGE_TRUSS_VERSION_PATCH 0

namespace bridge::detail::truss::cpp17::version {

/// @brief This struct holds Truss's version. You can read it at
///        compile time or at run time.
struct version_info {
    /// @brief This is the major version number. See @ref
    ///        BRIDGE_TRUSS_VERSION_MAJOR.
    std::uint32_t major = BRIDGE_TRUSS_VERSION_MAJOR;
    /// @brief This is the minor version number. See @ref
    ///        BRIDGE_TRUSS_VERSION_MINOR.
    std::uint32_t minor = BRIDGE_TRUSS_VERSION_MINOR;
    /// @brief This is the patch version number. See @ref
    ///        BRIDGE_TRUSS_VERSION_PATCH.
    std::uint32_t patch = BRIDGE_TRUSS_VERSION_PATCH;
};

/// @brief This namespace promotes `version_info` to
///        `bridge::exports::truss::version`.
namespace exports {
using bridge::detail::truss::cpp17::version::version_info;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::version

/// @brief This is the Exports namespace for `version`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
namespace bridge::exports::truss {
inline namespace cpp17 {
inline namespace version {
using namespace bridge::detail::truss::cpp17::version::exports;
} // namespace version
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief This is Truss's public API.
namespace bridge::truss {
using bridge::exports::truss::version_info;
} // namespace bridge::truss
