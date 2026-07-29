/// @file source_location.hpp
/// @brief This file holds Deck's `source_location`. This type acts
///        the same way in every ecosystem.
///
///        When the detected ecosystem has `std::source_location`,
///        this is a passthrough alias to it. When the ecosystem does
///        not have it yet, this is a plain alias to Truss's polyfill,
///        `bridge::truss::source_location`.
///
///        This file has the same shape as `deck/cpp17/span.hpp`.
///        Truss already owns a complete class for the pre-C++20 case
///        (docs/adr/0019-source-location-truss-owns-the-class.md).
///        Deck has nothing left to wrap, so this choice is a plain
///        type alias. Truss's class never itself passes through, even
///        under C++20. This file is the only place this choice
///        happens. See docs/adr/0001-namespace-and-export-scheme.md
///        for the namespace rule. See docs/adr/0008-best-effort-head-
///        standard.md for the bar this choice must clear.
///
/// This choice is a bare Feature Test check, not a Detector-backed
/// override. `BRIDGE_RIVETS_FEATURES_LIB_SOURCE_LOCATION` is a
/// reliable signal on every toolchain this project's matrix covers, a
/// direct compiler probe confirmed this before this file relied on
/// it, so this file needs no libstdc++-style override the way
/// `deck/cpp17/jthread.hpp` does.
#pragma once

#include <rivets/clang.hpp>
#include <rivets/diagnostics.hpp>
#include <rivets/features.hpp>

/// @def BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH
/// @brief This macro tells you whether `source_location` should pass
///        through to the real `std::source_location`. This macro is a
///        bare Feature Test check.
#define BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH (BRIDGE_RIVETS_FEATURES_LIB_SOURCE_LOCATION >= 201907L)

#if BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH
#    include <source_location>
#endif

#include <truss/cpp17/source_location.hpp>

namespace bridge::detail::deck::cpp17::source_location {

#if BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH

/// @brief This is Deck's passthrough choice. This ecosystem's
///        `std::source_location` is available. Bridge adds nothing
///        here.
using source_location = std::source_location;

#else

// docs/adr/0011-warn-on-surprising-facility-divergences.md: two
// disclosed divergences apply to this polyfill branch. The first
// (function_name()) always applies, confirmed by direct probe across
// this project's whole matrix. The second (column()) fires only when
// its own gap actually applies -- see its own #if just below.
BRIDGE_RIVETS_DIVERGENCE_NOTE(
    "bridge::source_location (polyfill): function_name() always returns a bare function name here (from __builtin_FUNCTION()), never a full signature. Real std::source_location, once available, reports a full function signature on libstdc++, the standard library this project's whole compiler matrix uses. See docs/adr/0019.")

// Clang (9 and later, this project's whole Clang matrix) gives
// Truss's polyfill a real column() through __builtin_COLUMN(). Only a
// non-Clang polyfill, chiefly GCC, has no portable builtin for this
// and always reports 0. Unlike function_name() above, this gap is a
// compiler split, not a standard-library-wide fact -- so this note is
// conditional, not unconditional. See docs/adr/0019 for the full
// rationale.
#if !BRIDGE_RIVETS_CLANG_GE(9)
BRIDGE_RIVETS_DIVERGENCE_NOTE(
    "bridge::source_location (polyfill): column() always returns 0 here -- this toolchain has no portable compiler builtin for a call site's column number. Real std::source_location, once available, reports the real column even on this same toolchain. See docs/adr/0019.")
#endif

/// @brief This is Truss's polyfill, for an ecosystem without
///        `std::source_location` yet.
using source_location = bridge::truss::source_location;

#endif

/// @brief This namespace promotes `source_location` to
///        `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::source_location::source_location;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::source_location

/// @brief This is the Exports namespace for `source_location`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace source_location { ... }`
/// wrapper, for the same reason as `deck/cpp17/stop_token.hpp`'s
/// Exports namespace. This header's primary export is a type named
/// `source_location`. The wrapper's name would be `source_location`
/// too, and the two names would collide. This namespace promotes
/// `source_location` straight from the `cpp17` inline namespace
/// instead, and avoids the collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::source_location::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief This is Deck's public API.
namespace bridge::deck {
using bridge::exports::deck::source_location;
} // namespace bridge::deck

/// @brief This is bridge's public API. This symbol reaches all the
///        way to `bridge::`, matching `span`'s own promotion chain.
namespace bridge {
using bridge::deck::source_location;
} // namespace bridge
