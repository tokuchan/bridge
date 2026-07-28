/// @file expected.hpp
/// @brief This file holds Deck's `expected<T,E>`. This type acts the
///        same way in every ecosystem.
///
///        When the detected ecosystem has `std::expected<T,E>`,
///        `expected<T,E>` is a passthrough alias to it. When the
///        ecosystem does not have `std::expected<T,E>` yet,
///        `expected<T,E>` is a plain alias to Truss's polyfill,
///        `bridge::truss::expected<T,E>`.
///
///        Unlike deck/cpp17/optional.hpp's wrapper, this choice is a
///        plain type alias. `optional`'s wrapper is built on Truss's
///        free functions, because `std::optional` predates C++17.
///        Truss already owns a complete class for the pre-C++23 case
///        here (docs/adr/0010-expected-truss-owns-the-class.md), so
///        Deck has nothing left to wrap. Truss's `expected<T,E>`
///        never itself passes through, even under C++23. This file
///        is the only place this choice happens. See
///        docs/adr/0001-namespace-and-export-scheme.md for the
///        namespace rule. See docs/adr/0008-best-effort-head-
///        standard.md for the bar this choice must clear, and see
///        docs/adr/0010 for the fidelity caveats it documents for
///        the polyfill path.
///
/// The same passthrough-or-polyfill choice applies to `unexpected`,
/// `unexpect_t`, `unexpect`, and `bad_expected_access` too, not just
/// `expected` itself. Hitting a real compile error first confirmed
/// this is necessary. Under passthrough, `bridge::expected` is
/// `std::expected`. Its constructors expect `std::unexpect_t` and
/// `std::unexpected<E>` specifically, not Truss's own types. Truss's
/// types are structurally identical, but distinct types. Hard-coding
/// those three to Truss's polyfill, regardless of which `expected`
/// was selected, would silently break construction the moment
/// passthrough activates. This is exactly the asymmetry
/// docs/adr/0008 exists to prevent.
///
/// The polyfill branch also carries two
/// `BRIDGE_RIVETS_DIVERGENCE_NOTE`s
/// (docs/adr/0011-warn-on-surprising-facility-divergences.md), for
/// disclosed divergences from real `std::expected` that only apply
/// there. See docs/adr/0010 for the full rationale behind each.
#pragma once

#include <rivets/diagnostics.hpp>
#include <rivets/features.hpp>

// <expected> doesn't exist at all before C++23 -- only include it when
// the Feature Test confirms this ecosystem actually has it, matching
// how the passthrough branch below is only ever selected in that case.
#if BRIDGE_RIVETS_FEATURES_LIB_EXPECTED >= 202211L
#    include <expected>
#endif

#include <truss/cpp17/expected.hpp>

namespace bridge::detail::deck::cpp17::expected {

#if BRIDGE_RIVETS_FEATURES_LIB_EXPECTED >= 202211L

/// @brief This is Deck's passthrough choice. This ecosystem's
///        `std::expected` is available. Bridge adds nothing here.
/// @see https://en.cppreference.com/w/cpp/utility/expected
template <class T, class E>
using expected = std::expected<T, E>;

/// @brief This is the passthrough companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
template <class E>
using unexpected = std::unexpected<E>;

/// @brief This is the passthrough companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
using unexpect_t = std::unexpect_t;
/// @brief This is the passthrough companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
inline constexpr unexpect_t unexpect = std::unexpect;

/// @brief This is the passthrough companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
template <class E>
using bad_expected_access = std::bad_expected_access<E>;

#else

// docs/adr/0011-warn-on-surprising-facility-divergences.md: two
// disclosed divergences from real std::expected apply whenever this
// polyfill branch is active. See docs/adr/0010-expected-truss-owns-
// the-class.md for the full rationale behind each.
BRIDGE_RIVETS_DIVERGENCE_NOTE(
    "bridge::expected (polyfill): converting constructors from unexpected<G>, a raw value, or expected<U,G> are unconditionally explicit here -- real std::expected allows implicit conversion when convertibility permits (C++17 has no explicit(bool) to make this conditional). See docs/adr/0010.")
BRIDGE_RIVETS_DIVERGENCE_NOTE(
    "bridge::expected (polyfill): the expected<U,G> converting constructor omits std::expected's extra defensive SFINAE guards (converts-from-any-cvref and friends), matching only the core is_constructible_v constraint -- an edge-case ambiguity real std::expected guards against that this polyfill doesn't. See docs/adr/0010.")

/// @brief This is Truss's polyfill, for an ecosystem without
///        `std::expected` yet.
/// @see https://en.cppreference.com/w/cpp/utility/expected
template <class T, class E>
using expected = bridge::truss::expected<T, E>;

/// @brief This is the polyfill companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
template <class E>
using unexpected = bridge::truss::unexpected<E>;

/// @brief This is the polyfill companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
using unexpect_t = bridge::truss::unexpect_t;
/// @brief This is the polyfill companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
inline constexpr unexpect_t unexpect = bridge::truss::unexpect;

/// @brief This is the polyfill companion to @ref expected.
/// @see https://en.cppreference.com/w/cpp/utility/expected
template <class E>
using bad_expected_access = bridge::truss::bad_expected_access<E>;

#endif

/// @brief This namespace promotes `expected`, `unexpected`,
///        `unexpect_t`, `unexpect`, and `bad_expected_access` to
///        `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::expected::expected;
using bridge::detail::deck::cpp17::expected::unexpected;
using bridge::detail::deck::cpp17::expected::unexpect_t;
using bridge::detail::deck::cpp17::expected::unexpect;
using bridge::detail::deck::cpp17::expected::bad_expected_access;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::expected

/// @brief This is the Exports namespace for `expected`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace expected { ... }`
/// wrapper, for the same reason as deck/cpp17/optional.hpp's Exports
/// namespace. This header's primary export is a type named
/// `expected`. The wrapper's name would be `expected` too, and the
/// two names would collide. This namespace promotes `expected`
/// straight from the `cpp17` inline namespace instead, and avoids
/// the collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::expected::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief This is Deck's public API.
namespace bridge::deck {
using bridge::exports::deck::expected;
using bridge::exports::deck::unexpected;
using bridge::exports::deck::unexpect_t;
using bridge::exports::deck::unexpect;
using bridge::exports::deck::bad_expected_access;
} // namespace bridge::deck

/// @brief This is bridge's public API. Every symbol here reaches all
///        the way to `bridge::`, matching `optional`'s own promotion
///        chain.
///
/// Unlike `optional`, where Truss's free functions operate on the
/// same `std::optional` regardless of path, so there is only ever
/// one set of symbols to promote, `expected`'s companion types come
/// from whichever path `bridge::deck::expected` itself selected.
/// Every companion symbol promotes from `bridge::deck::` here, not
/// from `bridge::truss::` directly, so they always match.
namespace bridge {
using bridge::deck::expected;
using bridge::deck::unexpected;
using bridge::deck::unexpect_t;
using bridge::deck::unexpect;
using bridge::deck::bad_expected_access;
} // namespace bridge
