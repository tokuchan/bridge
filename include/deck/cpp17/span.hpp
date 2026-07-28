/// @file span.hpp
/// @brief This file holds Deck's `span<T, Extent>`. This type acts
///        the same way in every ecosystem.
///
///        When the detected ecosystem has `std::span<T, Extent>`,
///        `span<T, Extent>` is a passthrough alias to it. When the
///        ecosystem does not have `std::span<T, Extent>` yet,
///        `span<T, Extent>` is a plain alias to Truss's polyfill,
///        `bridge::truss::span<T, Extent>`.
///
///        This file has the same shape as `deck/cpp17/expected.hpp`.
///        Truss already owns a complete class for the pre-C++20 case
///        (docs/adr/0015-span-truss-owns-the-class.md). Deck has
///        nothing left to wrap, so this choice is a plain type alias.
///        Truss's `span<T, Extent>` never itself passes through, even
///        under C++20. This file is the only place this choice
///        happens. See docs/adr/0001-namespace-and-export-scheme.md
///        for the namespace rule. See docs/adr/0008-best-effort-head-
///        standard.md for the bar this choice must clear.
///
/// The same passthrough-or-polyfill choice applies to
/// `dynamic_extent`, `as_bytes`, and `as_writable_bytes` too, not
/// just `span` itself. `expected`'s deck header needs the same choice
/// for `unexpected`, `unexpect_t`, and `bad_expected_access`. Under
/// passthrough, `bridge::span` is `std::span`. Its companions must be
/// the real ones too, not Truss's polyfilled versions, regardless of
/// which path Deck selects.
#pragma once

#include <rivets/features.hpp>

// <span> doesn't exist at all before C++20 -- only include it when the
// Feature Test confirms this ecosystem actually has it, matching how
// the passthrough branch below is only ever selected in that case.
#if BRIDGE_RIVETS_FEATURES_LIB_SPAN >= 202002L
#    include <span>
#endif

#include <truss/cpp17/span.hpp>

namespace bridge::detail::deck::cpp17::span {

#if BRIDGE_RIVETS_FEATURES_LIB_SPAN >= 202002L

/// @brief This is Deck's passthrough choice. This ecosystem's
///        `std::span` is already available. Bridge adds nothing
///        here.
template <class T, std::size_t Extent = std::dynamic_extent>
using span = std::span<T, Extent>;

/// @brief This is the passthrough companion to @ref span.
inline constexpr std::size_t dynamic_extent = std::dynamic_extent;

using std::as_bytes;
using std::as_writable_bytes;

#else

/// @brief This is Truss's polyfill, for an ecosystem without
///        `std::span` yet.
template <class T, std::size_t Extent = bridge::truss::dynamic_extent>
using span = bridge::truss::span<T, Extent>;

/// @brief This is the polyfill companion to @ref span.
inline constexpr std::size_t dynamic_extent = bridge::truss::dynamic_extent;

using bridge::truss::as_bytes;
using bridge::truss::as_writable_bytes;

#endif

/// @brief This namespace promotes `dynamic_extent`, `span`,
///        `as_bytes`, and `as_writable_bytes` to
///        `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::span::dynamic_extent;
using bridge::detail::deck::cpp17::span::span;
using bridge::detail::deck::cpp17::span::as_bytes;
using bridge::detail::deck::cpp17::span::as_writable_bytes;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::span

/// @brief This is the Exports namespace for `span`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace span { ... }` wrapper, for
/// the same reason as deck/cpp17/expected.hpp's Exports namespace.
/// This header's primary export is a type named `span`. The
/// wrapper's name would be `span` too, and the two names would
/// collide. This namespace promotes `span` straight from the `cpp17`
/// inline namespace instead, and avoids the collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::span::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief This is Deck's public API.
namespace bridge::deck {
using bridge::exports::deck::dynamic_extent;
using bridge::exports::deck::span;
using bridge::exports::deck::as_bytes;
using bridge::exports::deck::as_writable_bytes;
} // namespace bridge::deck

/// @brief This is bridge's public API. Every symbol here reaches all
///        the way to `bridge::`, matching `expected`'s own promotion
///        chain. Every companion symbol promotes from
///        `bridge::deck::` here, not from `bridge::truss::` directly.
///        This keeps every companion matching whichever path
///        `bridge::span` itself selected.
namespace bridge {
using bridge::deck::dynamic_extent;
using bridge::deck::span;
using bridge::deck::as_bytes;
using bridge::deck::as_writable_bytes;
} // namespace bridge
