/// @file print.hpp
/// @brief This file holds Truss's `std::print`/`std::println`
///        polyfill. This polyfill is built unconditionally on this
///        same library's `format.hpp`.
///
///        This polyfill is never built on whichever `format`
///        facility Deck ultimately selects. This preserves the
///        "Truss never passes through" rule uniformly across both
///        features. A benchmark confirmed this (docs/adr/0012): a
///        hypothetical native-`std::format`-backed alternative is
///        not worth the architectural exception it would require.
///        Truss's own `format` is measurably, but only modestly,
///        slower: about 15% to 30% slower per call, with both paths
///        well under 200ns. This is not the order-of-magnitude gap
///        that would justify an exception. See
///        docs/adr/0012-format-print-truss-owns-the-facility.md.
///
///        `bridge::truss::print` and `println` are always this
///        polyfill, regardless of standard or toolchain. Deck makes
///        that choice exactly once, independent of `format`'s own
///        choice. `format` and `print`/`println` cross their real
///        passthrough thresholds at different standards.
#pragma once

#include <cstdio>
#include <ostream>
#include <string>
#include <type_traits>
#include <utility>

#include <truss/cpp17/format.hpp>

namespace bridge::detail::truss::cpp17::print {

/// @brief This function writes `format(fmt, args...)` to `stream`.
/// @tparam Args The argument types.
/// @param stream The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/io/print
template <class... Args>
void print(std::FILE* stream, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    std::string s = bridge::truss::format(fmt, std::forward<Args>(args)...);
    std::fwrite(s.data(), 1, s.size(), stream);
}

/// @brief This function writes `format(fmt, args...)` to `stdout`.
/// @tparam Args The argument types.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/io/print
template <class... Args>
void print(bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    print(stdout, fmt, std::forward<Args>(args)...);
}

/// @brief This function writes `format(fmt, args...)` followed by a
///        newline to `stream`.
/// @tparam Args The argument types.
/// @param stream The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/io/print
template <class... Args>
void println(std::FILE* stream, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    print(stream, fmt, std::forward<Args>(args)...);
    std::fputc('\n', stream);
}

/// @brief This function writes `format(fmt, args...)` followed by a
///        newline to `stdout`.
/// @tparam Args The argument types.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/io/print
template <class... Args>
void println(bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    println(stdout, fmt, std::forward<Args>(args)...);
}

/// @brief This function writes `format(fmt, args...)` to `os`.
/// @tparam Args The argument types.
/// @param os The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/io/print
template <class... Args>
void print(std::ostream& os, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    std::string s = bridge::truss::format(fmt, std::forward<Args>(args)...);
    os.write(s.data(), static_cast<std::streamsize>(s.size()));
}

/// @brief This function writes `format(fmt, args...)` followed by a
///        newline to `os`.
/// @tparam Args The argument types.
/// @param os The destination stream.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @throws bridge::truss::format_error if `fmt` is malformed or
///         references an out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/io/print
template <class... Args>
void println(std::ostream& os, bridge::truss::format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    print(os, fmt, std::forward<Args>(args)...);
    os.put('\n');
}

/// @brief This namespace promotes `print` and `println` to
///        `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::print::print;
using bridge::detail::truss::cpp17::print::println;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::print

/// @brief This is the Exports namespace for `print`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace print { ... }` wrapper,
/// for the same reason as truss/cpp17/format.hpp's and
/// expected.hpp's Exports namespaces. This header's primary export
/// is a function named `print`. The wrapper's name would be `print`
/// too, and the two names would collide. This namespace promotes
/// straight from the `cpp17` inline namespace instead, and avoids
/// the collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::print::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief This is Truss's public API.
namespace bridge::truss {
using bridge::exports::truss::print;
using bridge::exports::truss::println;
} // namespace bridge::truss
