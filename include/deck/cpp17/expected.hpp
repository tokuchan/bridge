/// @file expected.hpp
/// @brief The STL-shaped `expected<T,E>` Deck owns: a passthrough alias
///        to `std::expected<T,E>` when the detected ecosystem has it,
///        or Truss's polyfilled `bridge::truss::expected<T,E>` when it
///        doesn't. Unlike deck/cpp17/optional.hpp's wrapper (built on
///        Truss's free functions since `std::optional` predates
///        C++17), this selection is a plain type alias -- Truss already
///        owns a complete class for the pre-C++23 case (docs/adr/0010-
///        expected-truss-owns-the-class.md), so there's nothing left
///        for Deck to wrap. Truss's `expected<T,E>` never itself passes
///        through, even under C++23; this is the only place the
///        selection happens. See docs/adr/0001-namespace-and-export-
///        scheme.md for the namespace scheme and docs/adr/0008-best-
///        effort-head-standard.md for the "behaviorally
///        indistinguishable" bar this selection has to clear (with the
///        fidelity caveats docs/adr/0010 documents for the polyfill
///        path).
///
/// The same passthrough-or-polyfill selection applies to `unexpected`,
/// `unexpect_t`/`unexpect`, and `bad_expected_access` too, not just
/// `expected` itself -- confirmed necessary by hitting a real compile
/// error first, not assumed: under passthrough, `bridge::expected` *is*
/// `std::expected`, whose constructors expect `std::unexpect_t`/
/// `std::unexpected<E>` specifically, not Truss's own (structurally
/// identical but distinct) types. Hard-coding those three to Truss's
/// polyfill regardless of which `expected` was selected would silently
/// break construction the moment passthrough activates -- exactly the
/// asymmetry docs/adr/0008 exists to prevent.
///
/// The polyfill branch also carries two `BRIDGE_RIVETS_DIVERGENCE_NOTE`s
/// (docs/adr/0011-warn-on-surprising-facility-divergences.md) for
/// disclosed divergences from real `std::expected` that only apply
/// there -- see docs/adr/0010 for the full rationale behind each.
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

/// @brief Passthrough: this ecosystem's `std::expected` is available,
///        so bridge adds nothing.
template <class T, class E>
using expected = std::expected<T, E>;

/// @brief Passthrough companion to @ref expected.
template <class E>
using unexpected = std::unexpected<E>;

/// @brief Passthrough companion to @ref expected.
using unexpect_t = std::unexpect_t;
/// @brief Passthrough companion to @ref expected.
inline constexpr unexpect_t unexpect = std::unexpect;

/// @brief Passthrough companion to @ref expected.
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

/// @brief Truss's polyfill, for ecosystems without native
///        `std::expected` yet.
template <class T, class E>
using expected = bridge::truss::expected<T, E>;

/// @brief Polyfill companion to @ref expected.
template <class E>
using unexpected = bridge::truss::unexpected<E>;

/// @brief Polyfill companion to @ref expected.
using unexpect_t = bridge::truss::unexpect_t;
/// @brief Polyfill companion to @ref expected.
inline constexpr unexpect_t unexpect = bridge::truss::unexpect;

/// @brief Polyfill companion to @ref expected.
template <class E>
using bad_expected_access = bridge::truss::bad_expected_access<E>;

#endif

/// @brief Symbols promoted to `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::expected::expected;
using bridge::detail::deck::cpp17::expected::unexpected;
using bridge::detail::deck::cpp17::expected::unexpect_t;
using bridge::detail::deck::cpp17::expected::unexpect;
using bridge::detail::deck::cpp17::expected::bad_expected_access;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::expected

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace expected { ... }` wrapper here (same reason as
/// deck/cpp17/optional.hpp's exports): this header's primary export is
/// a type named `expected`, and nesting it inside an inline namespace
/// of the identical name makes that inline namespace's own qualified
/// name reachable at this same scope, colliding with the promoted
/// type. Promoting straight from the `cpp17` inline namespace avoids
/// the collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::expected::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::expected;
using bridge::exports::deck::unexpected;
using bridge::exports::deck::unexpect_t;
using bridge::exports::deck::unexpect;
using bridge::exports::deck::bad_expected_access;
} // namespace bridge::deck

/// @brief Bridge's public API surface -- flattened all the way to
///        bridge::, matching optional's own promotion chain. Unlike
///        optional (where Truss's free functions operate on the same
///        std::optional regardless of path, so there's only ever one
///        set of symbols to promote), expected's companion types come
///        from whichever path bridge::deck::expected itself selected --
///        promoted from bridge::deck:: here, not bridge::truss::
///        directly, so they always match.
namespace bridge {
using bridge::deck::expected;
using bridge::deck::unexpected;
using bridge::deck::unexpect_t;
using bridge::deck::unexpect;
using bridge::deck::bad_expected_access;
} // namespace bridge
