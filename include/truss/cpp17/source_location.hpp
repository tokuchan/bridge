/// @file source_location.hpp
/// @brief This file holds Truss's `source_location` polyfill, for
///        standards that predate C++20. This polyfill is built from
///        scratch.
///
///        There is no C++17 type to attach free functions onto. Truss
///        owns a complete class here, the same way it does for
///        `expected`, `span`, and `stop_token`. See
///        docs/adr/0019-source-location-truss-owns-the-class.md for
///        the full decision, including two disclosed fidelity gaps.
///        First: GCC has never implemented a public
///        `__builtin_COLUMN()`. This polyfill's `column()` reports a
///        fixed `0` on GCC. This polyfill's `column()` reports the
///        real column on Clang. Second: this polyfill's
///        `function_name()` always reports a bare function name, from
///        `__builtin_FUNCTION()`. A direct probe confirmed real
///        `std::source_location::function_name()` reports a full
///        function signature instead, on libstdc++, the standard
///        library this project's whole compiler matrix uses. This
///        second gap is not a Clang-vs-GCC split, unlike the first.
///        It comes from the standard library, not the compiler.
///
/// `bridge::truss::source_location` is always this polyfill,
/// regardless of standard or toolchain. Truss never itself passes
/// through to `std::source_location`, even under C++20, where the
/// real type is available. Deck makes that choice exactly once
/// instead, in `deck/cpp17/source_location.hpp`. See
/// docs/adr/0001-namespace-and-export-scheme.md for the namespace
/// rule this file follows.
#pragma once

#include <cstdint>
#include <rivets/clang.hpp>

namespace bridge::detail::truss::cpp17::source_location {

/// @brief This class holds one point in source code: a file name, a
///        function name, a line number, and a column number. This
///        class matches `std::source_location`.
/// @see https://en.cppreference.com/w/cpp/utility/source_location
class source_location {
private:
    const char* file_;
    const char* func_;
    std::uint_least32_t line_;
    std::uint_least32_t column_;

public:
    /// @brief This builds a `source_location` for its caller's call
    ///        site. Call this function with no arguments. The default
    ///        arguments capture the call site through compiler
    ///        builtins.
    /// @param file The caller's file name. Leave this at its default.
    /// @param func The caller's enclosing function name. Leave this
    ///        at its default.
    /// @param line The caller's line number. Leave this at its
    ///        default.
    /// @param column The caller's column number. Leave this at its
    ///        default. This value is always `0` on GCC. GCC has no
    ///        public builtin for a call site's column number.
    /// @return A `source_location` for the call site.
    static constexpr source_location current(
        const char* file = __builtin_FILE(), const char* func = __builtin_FUNCTION(),
        std::uint_least32_t line = __builtin_LINE(),
#if BRIDGE_RIVETS_CLANG_GE(9)
        std::uint_least32_t column = __builtin_COLUMN()
#else
        std::uint_least32_t column = 0
#endif
            ) noexcept {
        source_location loc;
        loc.file_   = file;
        loc.func_   = func;
        loc.line_   = line;
        loc.column_ = column;
        return loc;
    }

    /// @brief This builds a `source_location` with no real call site.
    ///        `file_name()` and `function_name()` return an empty
    ///        string. `line()` and `column()` return `0`.
    constexpr source_location() noexcept : file_(""), func_(""), line_(0), column_(0) {}

    /// @brief This returns the file name this `source_location` holds.
    /// @return The file name, or an empty string for a
    ///         default-constructed `source_location`.
    constexpr const char* file_name() const noexcept { return file_; }

    /// @brief This returns the function name this `source_location`
    ///        holds. This name is bare, for example `foo`, not a full
    ///        signature. Real `std::source_location::function_name()`
    ///        returns a full signature on libstdc++, for example
    ///        `void foo()`.
    /// @return The function name, or an empty string for a
    ///         default-constructed `source_location`.
    constexpr const char* function_name() const noexcept { return func_; }

    /// @brief This returns the line number this `source_location`
    ///        holds.
    /// @return The line number, or `0` for a default-constructed
    ///         `source_location`.
    constexpr std::uint_least32_t line() const noexcept { return line_; }

    /// @brief This returns the column number this `source_location`
    ///        holds.
    /// @return The column number, or `0` for a default-constructed
    ///         `source_location`. This value is always `0` on GCC.
    constexpr std::uint_least32_t column() const noexcept { return column_; }
};

/// @brief This namespace promotes `source_location` to
///        `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::source_location::source_location;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::source_location

/// @brief This is the Exports namespace for `source_location`. See
///        docs/adr/0001-namespace-and-export-scheme.md for the rule
///        behind this namespace.
///
/// This namespace has no `inline namespace source_location { ... }`
/// wrapper, for the same reason as `truss/cpp17/stop_token.hpp`'s
/// Exports namespace. This header's primary export is a type named
/// `source_location`. The wrapper's name would be `source_location`
/// too, and the two names would collide. This namespace promotes
/// `source_location` straight from the `cpp17` inline namespace
/// instead, and avoids the collision.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::source_location::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief This is Truss's public API.
namespace bridge::truss {
using bridge::exports::truss::source_location;
} // namespace bridge::truss
