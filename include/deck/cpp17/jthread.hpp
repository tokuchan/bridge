/// @file jthread.hpp
/// @brief This file holds Deck's `jthread`. This type acts the same
///        way in every ecosystem.
///
///        When the detected ecosystem has `std::jthread`, `jthread`
///        is a passthrough alias to it. When the ecosystem does not
///        have `std::jthread` yet, `jthread` is a plain alias to
///        Truss's polyfill, `bridge::truss::jthread`.
///
///        This file has the same shape as `deck/cpp17/span.hpp`.
///        Truss already owns a complete class for the pre-C++20 case
///        (docs/adr/0017-jthread-stop-token-truss-owns-the-class.md).
///        Deck has nothing left to wrap, so this choice is a plain
///        type alias. Truss's `jthread` never itself passes through,
///        even under C++20. This file is the only place this choice
///        happens.
///
/// This file uses the exact same passthrough condition
/// `deck/cpp17/stop_token.hpp` does
/// (`BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH`), not a separate check.
/// `jthread` and `stop_token` must select the same path together. If
/// the two selections ever disagreed, a callable passed to
/// `bridge::jthread` that accepts a `bridge::stop_token` would be a
/// genuine type mismatch against real `std::jthread`, which only
/// ever passes `std::stop_token`. See docs/adr/0017 for the full
/// rationale.
#pragma once

#include <deck/cpp17/stop_token.hpp>

#if BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH
#    include <thread>
#endif

#include <truss/cpp17/jthread.hpp>

namespace bridge::detail::deck::cpp17::jthread {

#if BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH

/// @brief This is Deck's passthrough choice. This ecosystem's
///        `std::jthread` is available. This is the same condition
///        that selected passthrough for `bridge::stop_token`. Bridge
///        adds nothing here.
using jthread = std::jthread;

#else

/// @brief This is Truss's polyfill, for an ecosystem without a
///        reliably-signaled `std::jthread` yet.
using jthread = bridge::truss::jthread;

#endif

/// @brief This namespace promotes `jthread` to
///        `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::jthread::jthread;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::jthread

/// @brief This is the Exports namespace for `jthread`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace jthread { ... }` wrapper,
/// for the same reason as deck/cpp17/expected.hpp's Exports
/// namespace. This header's primary export is a type named
/// `jthread`. The wrapper's name would be `jthread` too, and the two
/// names would collide. This namespace promotes `jthread` straight
/// from the `cpp17` inline namespace instead, and avoids the
/// collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::jthread::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief This is Deck's public API.
namespace bridge::deck {
using bridge::exports::deck::jthread;
} // namespace bridge::deck

/// @brief This is bridge's public API. Every symbol here reaches all
///        the way to `bridge::`, matching `expected`'s own promotion
///        chain.
namespace bridge {
using bridge::deck::jthread;
} // namespace bridge
