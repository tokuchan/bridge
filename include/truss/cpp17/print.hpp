/// @file print.hpp
/// @brief Truss's `std::print`/`std::println` polyfill, built
///        unconditionally on this same library's `format.hpp` —
///        never on whichever `format` facility Deck ultimately
///        selects, preserving the "Truss never passes through"
///        invariant uniformly across both features. Confirmed via
///        benchmark (docs/adr/0012) that a hypothetical
///        native-`std::format`-backed alternative isn't worth the
///        architectural exception it would require: Truss's own
///        `format` is measurably but only modestly slower (~15-30%
///        per call, both paths well under 200ns), not the
///        order-of-magnitude gap that would justify one. See
///        docs/adr/0012-format-print-truss-owns-the-facility.md.
///
/// `bridge::truss::print`/`println` are unconditionally this polyfill,
/// regardless of standard or toolchain — that selection happens
/// exactly once, in Deck, independent of `format`'s own selection
/// (the two cross their real passthrough thresholds at different
/// standards).
#pragma once

#include <cstdio>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include <truss/cpp17/format.hpp>

namespace bridge::detail::truss::cpp17::print {

/// @brief Writes `format(fmt, args...)` to `stream`.
/// @tparam Args The argument types.
/// @param stream The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
template <class... Args>
void print(std::FILE* stream, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    std::string s = bridge::truss::format(fmt, std::forward<Args>(args)...);
    std::fwrite(s.data(), 1, s.size(), stream);
}

/// @brief Writes `format(fmt, args...)` to `stdout`.
/// @tparam Args The argument types.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
template <class... Args>
void print(bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    print(stdout, fmt, std::forward<Args>(args)...);
}

/// @brief Writes `format(fmt, args...)` followed by a newline to
///        `stream`.
/// @tparam Args The argument types.
/// @param stream The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
template <class... Args>
void println(std::FILE* stream, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    print(stream, fmt, std::forward<Args>(args)...);
    std::fputc('\n', stream);
}

/// @brief Writes `format(fmt, args...)` followed by a newline to
///        `stdout`.
/// @tparam Args The argument types.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
template <class... Args>
void println(bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    println(stdout, fmt, std::forward<Args>(args)...);
}

/// @brief Writes `format(fmt, args...)` to `os`.
/// @tparam Args The argument types.
/// @param os The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
template <class... Args>
void print(std::ostream& os, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    std::string s = bridge::truss::format(fmt, std::forward<Args>(args)...);
    os.write(s.data(), static_cast<std::streamsize>(s.size()));
}

/// @brief Writes `format(fmt, args...)` followed by a newline to `os`.
/// @tparam Args The argument types.
/// @param os The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
template <class... Args>
void println(std::ostream& os, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    print(os, fmt, std::forward<Args>(args)...);
    os.put('\n');
}

/// @brief Symbols promoted to `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::print::print;
using bridge::detail::truss::cpp17::print::println;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::print

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace print { ... }` wrapper here (same reason as
/// truss/cpp17/format.hpp's/expected.hpp's exports): this header's
/// primary export is a function named `print`, and nesting it inside
/// an inline namespace of the identical name makes that inline
/// namespace's own qualified name reachable at this same scope,
/// colliding with the promoted function. Promoting straight from the
/// `cpp17` inline namespace avoids the collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::print::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::print;
using bridge::exports::truss::println;
} // namespace bridge::truss
