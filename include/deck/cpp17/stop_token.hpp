/// @file stop_token.hpp
/// @brief This file holds Deck's `stop_token` and `stop_source`.
///        These types act the same way in every ecosystem.
///
///        When the detected ecosystem has `std::stop_token` and
///        `std::stop_source`, these are passthrough aliases to them.
///        When the ecosystem does not have them yet, these are plain
///        aliases to Truss's polyfill,
///        `bridge::truss::stop_token`/`stop_source`.
///
///        This file has the same shape as `deck/cpp17/span.hpp`.
///        Truss already owns complete classes for the pre-C++20 case
///        (docs/adr/0017-jthread-stop-token-truss-owns-the-class.md).
///        Deck has nothing left to wrap, so this choice is a plain
///        type alias. Truss's classes never themselves pass through,
///        even under C++20. This file is the only place this choice
///        happens. See docs/adr/0001-namespace-and-export-scheme.md
///        for the namespace rule. See docs/adr/0008-best-effort-head-
///        standard.md for the bar this choice must clear.
///
/// This choice is a Detector-backed override, not a bare Feature Test
/// check. `BRIDGE_RIVETS_FEATURES_LIB_JTHREAD` is always required.
/// This project's matrix confirmed this is a reliable signal on every
/// toolchain it covers. `BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN` is
/// honored normally, unless libstdc++ is the active standard library
/// (`bridge::rivets::libstdcxx`). When libstdc++ is active, this
/// facility never sees `BRIDGE_RIVETS_FEATURES_LIB_STOP_TOKEN`
/// published, even though `stop_token` itself works. A direct
/// compiler probe on GCC 13-15 and Clang 20 confirmed this, before
/// this file relied on it. See docs/adr/0017 for the full rationale.
///
/// `nostopstate_t` and `nostopstate` pass through the same way, not
/// just `stop_token` and `stop_source` themselves. `expected`'s deck
/// header needs the same choice for `unexpected` and `unexpect_t`.
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
/// @brief This macro tells you whether `stop_token`, `stop_source`,
///        and `jthread` should pass through to the real `std::`
///        types. This macro is a Detector-backed override, not a bare
///        Feature Test check. See
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

/// @brief This is Deck's passthrough choice. This ecosystem's
///        `std::stop_token` is available. This macro confirmed this
///        through `__cpp_lib_jthread`, with a Detector-backed
///        override for libstdc++'s `__cpp_lib_stop_token` gap.
///        Bridge adds nothing here.
using stop_token = std::stop_token;
/// @brief This is the passthrough companion to @ref stop_token.
using stop_source = std::stop_source;
/// @brief This is the passthrough companion to @ref stop_token.
using nostopstate_t = std::nostopstate_t;
/// @brief This is the passthrough companion to @ref stop_token.
inline constexpr nostopstate_t nostopstate = std::nostopstate;

#else

/// @brief This is Truss's polyfill, for an ecosystem without a
///        reliably-signaled `std::stop_token` yet.
using stop_token = bridge::truss::stop_token;
/// @brief This is the polyfill companion to @ref stop_token.
using stop_source = bridge::truss::stop_source;
/// @brief This is the polyfill companion to @ref stop_token.
using nostopstate_t = bridge::truss::nostopstate_t;
/// @brief This is the polyfill companion to @ref stop_token.
inline constexpr nostopstate_t nostopstate = bridge::truss::nostopstate;

#endif

/// @brief This namespace promotes `stop_token`, `stop_source`,
///        `nostopstate_t`, and `nostopstate` to
///        `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::stop_token::stop_token;
using bridge::detail::deck::cpp17::stop_token::stop_source;
using bridge::detail::deck::cpp17::stop_token::nostopstate_t;
using bridge::detail::deck::cpp17::stop_token::nostopstate;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::stop_token

/// @brief This is the Exports namespace for `stop_token`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace stop_token { ... }`
/// wrapper, for the same reason as deck/cpp17/expected.hpp's Exports
/// namespace. This header's primary export is a type named
/// `stop_token`. The wrapper's name would be `stop_token` too, and
/// the two names would collide. This namespace promotes `stop_token`
/// straight from the `cpp17` inline namespace instead, and avoids the
/// collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::stop_token::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief This is Deck's public API.
namespace bridge::deck {
using bridge::exports::deck::stop_token;
using bridge::exports::deck::stop_source;
using bridge::exports::deck::nostopstate_t;
using bridge::exports::deck::nostopstate;
} // namespace bridge::deck

/// @brief This is bridge's public API. Every symbol here reaches all
///        the way to `bridge::`, matching `expected`'s own promotion
///        chain. Every companion symbol promotes from
///        `bridge::deck::` here, not from `bridge::truss::` directly.
///        This keeps every companion matching whichever path
///        `bridge::stop_token` itself selected.
namespace bridge {
using bridge::deck::stop_token;
using bridge::deck::stop_source;
using bridge::deck::nostopstate_t;
using bridge::deck::nostopstate;
} // namespace bridge
