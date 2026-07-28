/// @file stop_token.hpp
/// @brief The STL-shaped `stop_token`/`stop_source` Deck owns: a
///        passthrough alias to `std::stop_token`/`std::stop_source`
///        when the detected ecosystem has them, or Truss's polyfilled
///        `bridge::truss::stop_token`/`stop_source` when it doesn't.
///        Same shape as `deck/cpp17/span.hpp`: Truss already owns
///        complete classes for the pre-C++20 case (docs/adr/0017-
///        jthread-stop-token-truss-owns-the-class.md), so there's
///        nothing left for Deck to wrap -- this selection is a plain
///        type alias. Truss's classes never themselves pass through,
///        even under C++20; this is the only place the selection
///        happens. See docs/adr/0001-namespace-and-export-scheme.md
///        for the namespace scheme and docs/adr/0008-best-effort-head-
///        standard.md for the "behaviorally indistinguishable" bar
///        this selection has to clear.
///
/// The passthrough condition is a Detector-backed override, not a bare
/// Feature Test check: `BRIDGE_RIVETS_FEATURES_LIB_JTHREAD` is always
/// required (the confirmed-reliable signal on every toolchain in this
/// project's matrix), and `BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN` is
/// honored normally *unless* libstdc++ is the active standard library
/// (`bridge::rivets::libstdcxx`), where it's never published even
/// though `stop_token` itself works -- confirmed by direct compiler
/// probe on GCC 13-15 and Clang 20 before writing this, not assumed.
/// See docs/adr/0017 for the full rationale. `nostopstate_t`/
/// `nostopstate` pass through the same way, not just `stop_token`/
/// `stop_source` themselves -- same reasoning `expected`'s deck header
/// needed for `unexpected`/`unexpect_t`.
#pragma once

#include <rivets/features.hpp>

// #ifndef BRIDGE_DOXYGEN, not a direct #include: Doxygen already
// documents rivets/libstdcxx.hpp via its own INPUT scan, and a second,
// indirect reference to a header that itself invokes
// BRIDGE_RIVETS_DEFINE_DETECTOR silently corrupts Doxygen's macro
// expansion for *unrelated* Named Detectors elsewhere (confirmed by
// bisection -- see Doxyfile.in's PREDEFINED comment). The real
// compiler always needs this include; Doxygen never defines
// BRIDGE_DOXYGEN, so it's unaffected either way.
#ifndef BRIDGE_DOXYGEN
#    include <rivets/libstdcxx.hpp>
#endif

/// @def BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH
/// @brief Whether `stop_token`/`stop_source`/`jthread` should pass
///        through to the real `std::` types. A Detector-backed
///        override, not a bare Feature Test check -- see
///        docs/adr/0017-jthread-stop-token-truss-owns-the-class.md.
#define BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH                                                                           \
    (BRIDGE_RIVETS_FEATURES_LIB_JTHREAD >= 201911L &&                                                                \
     (BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN >= 201907L || BRIDGE_RIVETS_LIBSTDCXX_GT(0)))

#if BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH
#    include <stop_token>
#endif

#include <truss/cpp17/stop_token.hpp>

namespace bridge::detail::deck::cpp17::stop_token {

#if BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH

/// @brief Passthrough: this ecosystem's `std::stop_token` is
///        available (confirmed via `__cpp_lib_jthread`, with a
///        Detector-backed override for libstdc++'s
///        `__cpp_lib_stop_token` gap), so bridge adds nothing.
using stop_token = std::stop_token;
/// @brief Passthrough companion to @ref stop_token.
using stop_source = std::stop_source;
/// @brief Passthrough companion to @ref stop_token.
using nostopstate_t = std::nostopstate_t;
/// @brief Passthrough companion to @ref stop_token.
inline constexpr nostopstate_t nostopstate = std::nostopstate;

#else

/// @brief Truss's polyfill, for ecosystems without a reliably-signaled
///        native `std::stop_token` yet.
using stop_token = bridge::truss::stop_token;
/// @brief Polyfill companion to @ref stop_token.
using stop_source = bridge::truss::stop_source;
/// @brief Polyfill companion to @ref stop_token.
using nostopstate_t = bridge::truss::nostopstate_t;
/// @brief Polyfill companion to @ref stop_token.
inline constexpr nostopstate_t nostopstate = bridge::truss::nostopstate;

#endif

/// @brief Symbols promoted to `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::stop_token::stop_token;
using bridge::detail::deck::cpp17::stop_token::stop_source;
using bridge::detail::deck::cpp17::stop_token::nostopstate_t;
using bridge::detail::deck::cpp17::stop_token::nostopstate;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::stop_token

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace stop_token { ... }` wrapper here (same reason
/// as deck/cpp17/expected.hpp's exports): this header's primary export
/// is a type named `stop_token`, and nesting it inside an inline
/// namespace of the identical name makes that inline namespace's own
/// qualified name reachable at this same scope, colliding with the
/// promoted type. Promoting straight from the `cpp17` inline namespace
/// avoids the collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::stop_token::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::stop_token;
using bridge::exports::deck::stop_source;
using bridge::exports::deck::nostopstate_t;
using bridge::exports::deck::nostopstate;
} // namespace bridge::deck

/// @brief Bridge's public API surface -- flattened all the way to
///        bridge::, matching expected's own promotion chain: every
///        companion symbol is promoted from bridge::deck:: here, not
///        bridge::truss:: directly, so they always match whichever
///        path bridge::stop_token itself selected.
namespace bridge {
using bridge::deck::stop_token;
using bridge::deck::stop_source;
using bridge::deck::nostopstate_t;
using bridge::deck::nostopstate;
} // namespace bridge
