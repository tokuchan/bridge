/// @file span.hpp
/// @brief The STL-shaped `span<T, Extent>` Deck owns: a passthrough
///        alias to `std::span<T, Extent>` when the detected ecosystem
///        has it, or Truss's polyfilled `bridge::truss::span<T, Extent>`
///        when it doesn't. Same shape as `deck/cpp17/expected.hpp`:
///        Truss already owns a complete class for the pre-C++20 case
///        (docs/adr/0015-span-truss-owns-the-class.md), so there's
///        nothing left for Deck to wrap -- this selection is a plain
///        type alias. Truss's `span<T,Extent>` never itself passes
///        through, even under C++20; this is the only place the
///        selection happens. See docs/adr/0001-namespace-and-export-
///        scheme.md for the namespace scheme and docs/adr/0008-best-
///        effort-head-standard.md for the "behaviorally
///        indistinguishable" bar this selection has to clear.
///
/// The same passthrough-or-polyfill selection applies to
/// `dynamic_extent`, `as_bytes`, and `as_writable_bytes` too, not just
/// `span` itself -- same reasoning `expected`'s deck header needed for
/// `unexpected`/`unexpect_t`/`bad_expected_access`: under passthrough,
/// `bridge::span` *is* `std::span`, so its companions need to be the
/// real ones too, not Truss's polyfilled versions regardless of path.
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

/// @brief Passthrough: this ecosystem's `std::span` is available, so
///        bridge adds nothing.
template <class T, std::size_t Extent = std::dynamic_extent>
using span = std::span<T, Extent>;

/// @brief Passthrough companion to @ref span.
inline constexpr std::size_t dynamic_extent = std::dynamic_extent;

using std::as_bytes;
using std::as_writable_bytes;

#else

/// @brief Truss's polyfill, for ecosystems without native `std::span`
///        yet.
template <class T, std::size_t Extent = bridge::truss::dynamic_extent>
using span = bridge::truss::span<T, Extent>;

/// @brief Polyfill companion to @ref span.
inline constexpr std::size_t dynamic_extent = bridge::truss::dynamic_extent;

using bridge::truss::as_bytes;
using bridge::truss::as_writable_bytes;

#endif

/// @brief Symbols promoted to `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::span::dynamic_extent;
using bridge::detail::deck::cpp17::span::span;
using bridge::detail::deck::cpp17::span::as_bytes;
using bridge::detail::deck::cpp17::span::as_writable_bytes;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::span

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace span { ... }` wrapper here (same reason as
/// deck/cpp17/expected.hpp's exports): this header's primary export is
/// a type named `span`, and nesting it inside an inline namespace of
/// the identical name makes that inline namespace's own qualified name
/// reachable at this same scope, colliding with the promoted type.
/// Promoting straight from the `cpp17` inline namespace avoids the
/// collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::span::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::dynamic_extent;
using bridge::exports::deck::span;
using bridge::exports::deck::as_bytes;
using bridge::exports::deck::as_writable_bytes;
} // namespace bridge::deck

/// @brief Bridge's public API surface -- flattened all the way to
///        bridge::, matching expected's own promotion chain: every
///        companion symbol is promoted from bridge::deck:: here, not
///        bridge::truss:: directly, so they always match whichever
///        path bridge::span itself selected.
namespace bridge {
using bridge::deck::dynamic_extent;
using bridge::deck::span;
using bridge::deck::as_bytes;
using bridge::deck::as_writable_bytes;
} // namespace bridge
