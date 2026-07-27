/// @file format.hpp
/// @brief The `format`/`format_to`/`format_to_n`/`formatted_size`/
///        `vformat` family Deck owns: passthrough aliases to real
///        `std::format`/etc. when the detected ecosystem has them, or
///        Truss's polyfilled `bridge::truss::format`/etc. otherwise.
///        Truss owns the whole facility outright (there's no
///        pre-existing STL shape to extend, unlike `optional`) --
///        docs/adr/0012-format-print-truss-owns-the-facility.md -- so,
///        same as deck/cpp17/expected.hpp, this selection is a plain
///        set of aliases with nothing left for Deck to build. Truss's
///        `format` never itself passes through, even under C++20; this
///        is the only place the selection happens. See docs/adr/0001-
///        namespace-and-export-scheme.md for the namespace scheme and
///        docs/adr/0008-best-effort-head-standard.md for the
///        "behaviorally indistinguishable" bar this selection has to
///        clear.
///
/// The same passthrough-or-polyfill selection applies to every
/// companion symbol a `formatter<T>` specialization or a `format_to`
/// call might need to name -- `format_error`, `format_parse_context`,
/// `format_context`, `formatter`, `format_string`, `format_to_n_result`,
/// `format_args`, `make_format_args` -- not just the entry-point
/// functions, mirroring why deck/cpp17/expected.hpp had to
/// dual-select `unexpected`/`unexpect_t`/`bad_expected_access`
/// alongside `expected` itself.
///
/// **`formatter<T>` is a special case.** It's the one symbol here
/// users *extend* (by specializing it for their own types), not just
/// name. C++ doesn't allow specializing an alias template
/// ([temp.alias]), so `bridge::formatter<T>` below -- an alias
/// template on both branches -- can be *named* (e.g. in a non-template
/// `formatter<T>::format` override that only wants to support one
/// context type) but never *specialized* directly. A user extending
/// formatting for their own type must specialize the real underlying
/// template instead: `std::formatter<T>` under passthrough,
/// `bridge::truss::formatter<T>` under polyfill -- and a type meant to
/// stay formattable across the passthrough boundary (e.g. a toolchain
/// upgrade from C++17 to C++20) genuinely needs *both*
/// specializations, since the two engines look up formatters in
/// different namespaces and C++17 has no `std::formatter` to unify
/// toward. This is a real, inherent limitation of the language, not a
/// gap this project's design failed to close -- see the
/// `BRIDGE_RIVETS_DIVERGENCE_NOTE` on the polyfill branch below and
/// docs/adr/0012's "formatter<T> is not transparently unifiable"
/// section.
#pragma once

#include <rivets/diagnostics.hpp>
#include <rivets/features.hpp>

// <format> doesn't exist at all before C++20 -- only include it when
// the Feature Test confirms this ecosystem actually has it, matching
// how the passthrough branch below is only ever selected in that case.
#if BRIDGE_RIVETS_FEATURES_LIB_FORMAT >= 201907L
#    include <format>
#endif

#include <truss/cpp17/format.hpp>

namespace bridge::detail::deck::cpp17::format {

#if BRIDGE_RIVETS_FEATURES_LIB_FORMAT >= 201907L

/// @brief Passthrough: this ecosystem's `std::format_error` is
///        available, so bridge adds nothing.
/// @see https://en.cppreference.com/w/cpp/utility/format
using format_error = std::format_error;

/// @brief Passthrough companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
using format_parse_context = std::format_parse_context;

/// @brief Passthrough companion to `format`. Named for
///        completeness (e.g. a non-generic `formatter<T>::format`
///        override) -- ordinary code names `FormatContext` via a
///        template parameter instead, matching how real `formatter`
///        specializations are written.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class OutIt>
using format_context = std::basic_format_context<OutIt, char>;

/// @brief Passthrough companion to `format`. An alias template --
///        cannot be specialized directly; see this file's top-of-file
///        doc comment.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class T>
using formatter = std::formatter<T>;

/// @brief Passthrough companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class... Args>
using format_string = std::format_string<Args...>;

/// @brief Passthrough companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class OutIt>
using format_to_n_result = std::format_to_n_result<OutIt>;

/// @brief Passthrough companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
using format_args = std::format_args;

using std::format;
using std::format_to;
using std::format_to_n;
using std::formatted_size;
using std::make_format_args;
using std::vformat;

#else

// docs/adr/0011-warn-on-surprising-facility-divergences.md: a user who
// specializes bridge::truss::formatter<T> under this polyfill branch
// (the only way to extend bridge::format for their own type here) may
// reasonably expect that same specialization to keep working once this
// build starts using native std::format instead -- it won't. See
// docs/adr/0012.
///
/// \cond BRIDGE_DETAIL
BRIDGE_RIVETS_DIVERGENCE_NOTE(
    "bridge::formatter (polyfill): specializing bridge::truss::formatter<T> here makes T formattable via bridge::format on this toolchain, but is NOT picked up once native std::format passthrough activates (e.g. after a C++17->C++20 upgrade) -- that path looks up std::formatter<T> instead, in a different namespace. A type meant to stay formattable across that boundary needs both specializations. See docs/adr/0012.")
/// \endcond

/// @brief Truss's polyfill, for ecosystems without native
///        `std::format` yet.
/// @see https://en.cppreference.com/w/cpp/utility/format
using format_error = bridge::truss::format_error;

/// @brief Polyfill companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
using format_parse_context = bridge::truss::format_parse_context;

/// @brief Polyfill companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class OutIt>
using format_context = bridge::truss::format_context<OutIt>;

/// @brief Polyfill companion to `format`. An alias template --
///        cannot be specialized directly; see this file's top-of-file
///        doc comment.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class T>
using formatter = bridge::truss::formatter<T>;

/// @brief Polyfill companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class... Args>
using format_string = bridge::truss::format_string<Args...>;

/// @brief Polyfill companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class OutIt>
using format_to_n_result = bridge::truss::format_to_n_result<OutIt>;

/// @brief Polyfill companion to `format`.
/// @see https://en.cppreference.com/w/cpp/utility/format
using format_args = bridge::truss::format_args;

using bridge::truss::format;
using bridge::truss::format_to;
using bridge::truss::format_to_n;
using bridge::truss::formatted_size;
using bridge::truss::make_format_args;
using bridge::truss::vformat;

#endif

/// @brief Symbols promoted to `bridge::exports::deck`.
namespace exports {
using bridge::detail::deck::cpp17::format::format_error;
using bridge::detail::deck::cpp17::format::format_parse_context;
using bridge::detail::deck::cpp17::format::format_context;
using bridge::detail::deck::cpp17::format::formatter;
using bridge::detail::deck::cpp17::format::format_string;
using bridge::detail::deck::cpp17::format::format_to_n_result;
using bridge::detail::deck::cpp17::format::format_args;
using bridge::detail::deck::cpp17::format::format;
using bridge::detail::deck::cpp17::format::format_to;
using bridge::detail::deck::cpp17::format::format_to_n;
using bridge::detail::deck::cpp17::format::formatted_size;
using bridge::detail::deck::cpp17::format::make_format_args;
using bridge::detail::deck::cpp17::format::vformat;
} // namespace exports

} // namespace bridge::detail::deck::cpp17::format

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace format { ... }` wrapper here (same reason as
/// truss/cpp17/format.hpp's own exports): this header's primary export
/// is a function named `format`, and nesting it inside an inline
/// namespace of the identical name makes that inline namespace's own
/// qualified name reachable at this same scope, colliding with the
/// promoted function. Promoting straight from the `cpp17` inline
/// namespace avoids the collision.
namespace bridge::exports::deck {
inline namespace cpp17 {
using namespace bridge::detail::deck::cpp17::format::exports;
} // namespace cpp17
} // namespace bridge::exports::deck

/// @brief Deck's public API surface.
namespace bridge::deck {
using bridge::exports::deck::format_error;
using bridge::exports::deck::format_parse_context;
using bridge::exports::deck::format_context;
using bridge::exports::deck::formatter;
using bridge::exports::deck::format_string;
using bridge::exports::deck::format_to_n_result;
using bridge::exports::deck::format_args;
using bridge::exports::deck::format;
using bridge::exports::deck::format_to;
using bridge::exports::deck::format_to_n;
using bridge::exports::deck::formatted_size;
using bridge::exports::deck::make_format_args;
using bridge::exports::deck::vformat;
} // namespace bridge::deck

/// @brief Bridge's public API surface -- flattened all the way to
///        bridge::, matching expected's own promotion chain: every
///        companion symbol is promoted from bridge::deck:: here, not
///        bridge::truss:: directly, so they always match whichever
///        path bridge::format itself selected.
namespace bridge {
using bridge::deck::format_error;
using bridge::deck::format_parse_context;
using bridge::deck::format_context;
using bridge::deck::formatter;
using bridge::deck::format_string;
using bridge::deck::format_to_n_result;
using bridge::deck::format_args;
using bridge::deck::format;
using bridge::deck::format_to;
using bridge::deck::format_to_n;
using bridge::deck::formatted_size;
using bridge::deck::make_format_args;
using bridge::deck::vformat;
} // namespace bridge
