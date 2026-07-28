/// @file jthread.hpp
/// @brief The STL-shaped `jthread` Deck owns: a passthrough alias to
///        `std::jthread` when the detected ecosystem has it, or
///        Truss's polyfilled `bridge::truss::jthread` when it doesn't.
///        Same shape as `deck/cpp17/span.hpp`: Truss already owns a
///        complete class for the pre-C++20 case (docs/adr/0017-
///        jthread-stop-token-truss-owns-the-class.md), so there's
///        nothing left for Deck to wrap -- this selection is a plain
///        type alias. Truss's `jthread` never itself passes through,
///        even under C++20; this is the only place the selection
///        happens.
///
/// Uses the exact same passthrough condition `deck/cpp17/stop_token.hpp`
/// does (`BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH`), not a separate check --
/// `jthread` and `stop_token` must select the same path *together*: a
/// callable passed to `bridge::jthread` that accepts a
/// `bridge::stop_token` would be a genuine type mismatch against real
/// `std::jthread` (which only ever passes `std::stop_token`) if the two
/// selections ever disagreed. See docs/adr/0017 for the full rationale.
#pragma once

#include <deck/cpp17/stop_token.hpp>

#if BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH
#    include <thread>
#endif

#include <truss/cpp17/jthread.hpp>

namespace bridge::detail::deck::cpp17::jthread {

#if BRIDGE_DECK_STOP_TOKEN_PASSTHROUGH

/// @brief Passthrough: this ecosystem's `std::jthread` is available
///        (same condition `bridge::stop_token` selected passthrough
///        on), so bridge adds nothing.
using jthread = std::jthread;

#else

/// @brief Truss's polyfill, for ecosystems without a reliably-signaled
///        native `std::jthread` yet.
using jthread = bridge::truss::jthread;

#endif

/// @brief Symbols promoted to `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::jthread::jthread;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::jthread

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace jthread { ... }` wrapper here (same reason as
/// deck/cpp17/expected.hpp's exports): this header's primary export is
/// a type named `jthread`, and nesting it inside an inline namespace of
/// the identical name makes that inline namespace's own qualified name
/// reachable at this same scope, colliding with the promoted type.
/// Promoting straight from the `cpp17` inline namespace avoids the
/// collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::jthread::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::jthread;
} // namespace bridge::deck

/// @brief Bridge's public API surface -- flattened all the way to
///        bridge::, matching expected's own promotion chain.
namespace bridge {
using bridge::deck::jthread;
} // namespace bridge
