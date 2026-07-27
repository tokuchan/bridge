/// @file print.hpp
/// @brief The `print`/`println` family Deck owns: passthrough aliases
///        to real `std::print`/`std::println` when the detected
///        ecosystem has them, or Truss's polyfilled
///        `bridge::truss::print`/`println` otherwise. Gated on
///        `BRIDGE_RIVETS_FEATURES_LIB_PRINT` independently of
///        `format.hpp`'s own `BRIDGE_RIVETS_FEATURES_LIB_FORMAT` gate
///        -- confirmed by direct compiler probe (docs/adr/0012) that
///        `format` and `print` cross their real passthrough
///        thresholds at different standards (C++20 vs C++23), so an
///        ecosystem can have one without the other.
///
/// Both the `FILE*`-targeting and `ostream`-targeting overload
/// families are aliased here -- confirmed by direct compiler probe
/// that real `std::print`/`std::println` provide both (P2093), not
/// just the `FILE*` family, so there's nothing bridge-specific to
/// reconcile between the two paths.
///
/// Same "Truss never passes through, only Deck selects" invariant as
/// `format.hpp`/`expected.hpp`; unlike `format.hpp`, `print`/`println`
/// have no user-extensible customization point, so this header is a
/// plain set of `using` aliases with none of `formatter<T>`'s
/// alias-template caveat.
#pragma once

#include <rivets/features.hpp>

// <print> doesn't exist at all before C++23 -- only include it when
// the Feature Test confirms this ecosystem actually has it, matching
// how the passthrough branch below is only ever selected in that case.
// <ostream> is included explicitly alongside it: `using std::print;`/
// `using std::println;` below freeze the overload set at this point,
// and the ostream-targeting overloads live in <ostream>, not <print>
// -- included here defensively rather than relying on it arriving
// transitively via truss/cpp17/print.hpp's own includes.
#if BRIDGE_RIVETS_FEATURES_LIB_PRINT >= 202211L
#    include <ostream>
#    include <print>
#endif

#include <truss/cpp17/print.hpp>

namespace bridge::detail::deck::cpp17::print {

#if BRIDGE_RIVETS_FEATURES_LIB_PRINT >= 202211L

using std::print;
using std::println;

#else

using bridge::truss::print;
using bridge::truss::println;

#endif

/// @brief Symbols promoted to `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::print::print;
using bridge::detail::deck::cpp17::print::println;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::print

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace print { ... }` wrapper here (same reason as
/// truss/cpp17/print.hpp's/format.hpp's exports): this header's
/// primary export is a function named `print`, and nesting it inside
/// an inline namespace of the identical name makes that inline
/// namespace's own qualified name reachable at this same scope,
/// colliding with the promoted function. Promoting straight from the
/// `cpp17` inline namespace avoids the collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::print::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::print;
using bridge::exports::deck::println;
} // namespace bridge::deck

/// @brief Bridge's public API surface -- flattened all the way to
///        bridge::, matching format's/expected's own promotion chain.
namespace bridge {
using bridge::deck::print;
using bridge::deck::println;
} // namespace bridge
