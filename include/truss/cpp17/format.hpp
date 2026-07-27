/// @file format.hpp
/// @brief Truss's from-scratch `std::format`/`std::print` polyfill for
///        standards that predate C++20/23. Unlike `optional` (free
///        functions on an existing type) or `expected` (a full class,
///        since no existing type to extend), `format` is
///        function-shaped with no pre-existing STL facility at all
///        pre-C++20 — Truss owns the whole thing: this file's the
///        parser, the contexts, the `formatter<T>` customization point
///        and its built-in specializations, and the top-level
///        `format`/`format_to`/`format_to_n`/`formatted_size`/`vformat`
///        entry points. `print`/`println` (truss/cpp17/print.hpp) are
///        built on this file unconditionally, never on whichever
///        format facility Deck ultimately selects. See
///        docs/adr/0012-format-print-truss-owns-the-facility.md for
///        the full rationale and disclosed scope limitations,
///        docs/adr/0001-namespace-and-export-scheme.md for the
///        namespace scheme, and docs/adr/0011-warn-on-surprising-
///        facility-divergences.md for how those limitations are
///        surfaced to a consumer actually on the polyfill path.
///
/// `bridge::truss::format`/`format_to`/etc. are unconditionally this
/// polyfill, regardless of standard or toolchain — Truss never itself
/// passes through to `std::format`, even under C++20+ where the real
/// facility is available. That selection happens exactly once, in Deck.
#pragma once

#include <cstddef>
#include <functional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace bridge::detail::truss::cpp17::format {

/// @brief Thrown when a format string is malformed, references an
///        out-of-range or type-mismatched argument, or otherwise
///        can't be honored. Matches `std::format_error`.
class format_error : public std::runtime_error {
public:
    /// @brief Constructs from a `std::string` message.
    /// @param what_arg The error message.
    explicit format_error(const std::string& what_arg) : std::runtime_error(what_arg) {}
    /// @brief Constructs from a C-string message.
    /// @param what_arg The error message.
    explicit format_error(const char* what_arg) : std::runtime_error(what_arg) {}
};

/// @brief Cursor over the not-yet-consumed portion of a replacement
///        field's format-spec substring, plus the auto/manual
///        argument-indexing state shared across the whole format
///        string. Matches real `std::format_parse_context`'s
///        `begin()`/`end()`/`advance_to()`/`next_arg_id()`/
///        `check_arg_id()` shape — a user's `formatter<T>::parse`
///        written against this type is source-compatible with the
///        real one. Not `constexpr` here (unlike the real type): this
///        polyfill never evaluates format strings at compile time in
///        the first place (no `consteval` pre-C++20 to do so with —
///        see docs/adr/0012's disclosed compile-time-validation gap),
///        so there's nothing to gain from it.
class format_parse_context {
public:
    /// @brief The character type. Always `char` — this polyfill is
    ///        narrow-char/UTF-8 only (docs/adr/0012).
    using char_type = char;
    /// @brief The iterator type over the remaining spec substring.
    using const_iterator = std::string_view::const_iterator;
    /// @copydoc const_iterator
    using iterator = const_iterator;

    /// @brief Constructs a context over `fmt`.
    /// @param fmt The full format string (or, when constructed
    ///        per-field, the spec substring for one field).
    /// @param num_args The number of format arguments available, for
    ///        diagnostic purposes.
    explicit format_parse_context(std::string_view fmt, std::size_t num_args = 0) noexcept
        : fmt_(fmt), num_args_(num_args) {}
    /// @brief Never copied, matching the real type.
    format_parse_context(const format_parse_context&) = delete;
    /// @brief Never copied, matching the real type.
    /// @return `*this`.
    format_parse_context& operator=(const format_parse_context&) = delete;

    /// @brief The start of the not-yet-consumed spec substring.
    /// @return An iterator to the first unconsumed character.
    const_iterator begin() const noexcept { return fmt_.begin(); }
    /// @brief One past the last character of the spec substring.
    /// @return An iterator one past the last character.
    const_iterator end() const noexcept { return fmt_.end(); }
    /// @brief Marks everything before `it` as consumed.
    /// @param it An iterator previously obtained from this context.
    void advance_to(const_iterator it) { fmt_ = fmt_.substr(static_cast<std::size_t>(it - fmt_.begin())); }

    /// @brief Allocates the next automatic argument index. Throws if
    ///        this format string has already used an explicit
    ///        (manual) argument index — matching real
    ///        `std::format_parse_context`'s "cannot mix" rule.
    /// @return The next automatic argument index, starting from 0.
    /// @throws format_error if manual indexing was already used.
    std::size_t next_arg_id() {
        if (indexing_ == indexing_mode::manual) {
            throw format_error("cannot switch from manual to automatic argument indexing");
        }
        indexing_ = indexing_mode::automatic;
        return next_id_++;
    }
    /// @brief Records that `id` was used as an explicit (manual)
    ///        argument index. Throws if this format string has
    ///        already used automatic indexing.
    /// @param id The explicit argument index just parsed (unused
    ///        beyond the indexing-mode check; out-of-range checking
    ///        happens where the argument is actually fetched).
    /// @throws format_error if automatic indexing was already used.
    void check_arg_id(std::size_t id) {
        (void)id;
        if (indexing_ == indexing_mode::automatic) {
            throw format_error("cannot switch from automatic to manual argument indexing");
        }
        indexing_ = indexing_mode::manual;
    }

    /// @brief The number of format arguments available.
    /// @return The argument count passed at construction.
    std::size_t num_args() const noexcept { return num_args_; }

private:
    enum class indexing_mode { unknown, automatic, manual };
    std::string_view fmt_;
    std::size_t num_args_;
    std::size_t next_id_ = 0;
    indexing_mode indexing_ = indexing_mode::unknown;
};

/// @brief Fill-and-align alignment, from a parsed format-spec.
enum class align_t : unsigned char {
    none,  ///< No alignment specified; the formatter picks its own default.
    left,  ///< `<`
    right, ///< `>`
    center ///< `^`
};

/// @brief Sign handling, from a parsed format-spec.
enum class sign_t : unsigned char {
    minus, ///< `-` (default): sign shown only for negative values.
    plus,  ///< `+`: sign always shown.
    space  ///< ` `: space for non-negative, `-` for negative.
};

/// @brief A parsed width or precision: either absent, a literal value,
///        or a reference to another argument (`{}`/`{N}`) resolved at
///        format time via @ref dynamic_arg_source.
struct width_or_precision {
    /// @brief Which alternative this holds.
    enum class kind : unsigned char {
        none,    ///< Not specified.
        literal, ///< `value` is the literal width/precision.
        dynamic  ///< `value` is the argument index to resolve at format time.
    };
    /// @copydoc kind
    kind k = kind::none;
    /// @brief The literal value, or the dynamic argument index,
    ///        depending on @ref k.
    std::size_t value = 0;
};

/// @brief The fully-parsed standard format-spec grammar (everything
///        `parse_std_spec` understands): `[[fill]align][sign]["#"]
///        ["0"][width]["." precision]["L"][type]`. `type` is left
///        `'\0'` when the spec has none (the field's default
///        presentation applies).
struct parsed_std_spec {
    /// @brief The fill character. `' '` (space) when unspecified.
    char fill = ' ';
    /// @brief The alignment. `align_t::none` when unspecified.
    align_t align = align_t::none;
    /// @brief The sign handling. `sign_t::minus` (the default) when unspecified.
    sign_t sign = sign_t::minus;
    /// @brief Whether `#` (alternate form) was present.
    bool alt = false;
    /// @brief Whether `0` (zero-padding) was present.
    bool zero_pad = false;
    /// @brief The parsed width, if any.
    width_or_precision width{};
    /// @brief The parsed precision, if any.
    width_or_precision precision{};
    /// @brief Whether `L` (locale-aware formatting) was present.
    ///        Accepted syntactically but always ignored — behaves as
    ///        if absent, matching docs/adr/0012's disclosed scope
    ///        exclusion for locale support.
    bool locale = false;
    /// @brief The trailing presentation-type character (e.g. `'d'`,
    ///        `'x'`, `'f'`), or `'\0'` if the spec had none.
    char type = '\0';
};

/// @brief Maps `'<'`/`'>'`/`'^'` to `align_t`. Precondition: `c` is
///        one of those three characters.
/// @param c The alignment character.
/// @return The matching `align_t`.
inline align_t align_from_char(char c) {
    if (c == '<') return align_t::left;
    if (c == '>') return align_t::right;
    return align_t::center;
}

/// @brief Parses a `width` or `precision` production (a literal
///        decimal integer, or `{}`/`{N}` referencing another
///        argument), starting at `pctx`'s current position. Advances
///        `pctx` past what was consumed.
/// @param pctx The parse context, positioned at the first character of
///        the width/precision production.
/// @param allow_zero_literal Whether a literal `0` is acceptable
///        (`false` for width, matching the standard's "positive
///        decimal integer" rule for width literals; `true` for
///        precision, which permits `0`).
/// @return The parsed width/precision.
/// @throws format_error if the production is malformed.
inline width_or_precision parse_width_or_precision_value(format_parse_context& pctx, bool allow_zero_literal) {
    auto it = pctx.begin();
    auto end = pctx.end();
    width_or_precision result;
    if (it != end && *it == '{') {
        ++it;
        if (it != end && *it == '}') {
            result.k = width_or_precision::kind::dynamic;
            pctx.advance_to(it);
            result.value = pctx.next_arg_id();
            it = pctx.begin();
            if (it == end || *it != '}') throw format_error("expected '}' after dynamic width/precision");
            ++it;
        } else {
            std::size_t id = 0;
            bool any = false;
            while (it != end && *it >= '0' && *it <= '9') {
                any = true;
                id = id * 10 + static_cast<std::size_t>(*it - '0');
                ++it;
            }
            if (!any || it == end || *it != '}') throw format_error("invalid dynamic width/precision reference");
            result.k = width_or_precision::kind::dynamic;
            result.value = id;
            pctx.check_arg_id(id);
            ++it;
        }
        pctx.advance_to(it);
    } else {
        std::size_t val = 0;
        bool any = false;
        while (it != end && *it >= '0' && *it <= '9') {
            any = true;
            val = val * 10 + static_cast<std::size_t>(*it - '0');
            ++it;
        }
        if (!any) throw format_error("expected width/precision digits");
        if (!allow_zero_literal && val == 0) throw format_error("width must be nonzero");
        result.k = width_or_precision::kind::literal;
        result.value = val;
        pctx.advance_to(it);
    }
    return result;
}

/// @brief Parses the full standard format-spec grammar (fill-and-
///        align, sign, `#`, `0`, width, precision, `L`, type) starting
///        at `pctx`'s current position. Advances `pctx` past
///        everything consumed, leaving it positioned at the field's
///        closing `}` (the caller — the top-level format engine —
///        validates and consumes that itself).
/// @param pctx The parse context, positioned at the start of the spec.
/// @param allowed_types The presentation-type characters this
///        argument's formatter accepts (e.g. `"dbxX"` for integers);
///        an empty spec (no type character) is always allowed.
/// @return The parsed spec.
/// @throws format_error if the spec is malformed, or its type
///         character isn't in `allowed_types`.
inline parsed_std_spec parse_std_spec(format_parse_context& pctx, std::string_view allowed_types) {
    parsed_std_spec spec;
    auto it = pctx.begin();
    auto end = pctx.end();

    auto is_align_char = [](char c) { return c == '<' || c == '>' || c == '^'; };

    if (it != end && *it != '{' && *it != '}') {
        auto next = it + 1;
        if (next != end && is_align_char(*next)) {
            spec.fill = *it;
            spec.align = align_from_char(*next);
            it += 2;
        } else if (is_align_char(*it)) {
            spec.align = align_from_char(*it);
            ++it;
        }
    }
    pctx.advance_to(it);

    it = pctx.begin();
    if (it != end && (*it == '+' || *it == '-' || *it == ' ')) {
        spec.sign = (*it == '+') ? sign_t::plus : (*it == ' ') ? sign_t::space : sign_t::minus;
        ++it;
        pctx.advance_to(it);
    }

    it = pctx.begin();
    if (it != end && *it == '#') {
        spec.alt = true;
        ++it;
        pctx.advance_to(it);
    }

    it = pctx.begin();
    if (it != end && *it == '0') {
        spec.zero_pad = true;
        ++it;
        pctx.advance_to(it);
    }

    // Includes '0' as a valid width-production start (not just '1'-'9')
    // despite width ultimately requiring a *nonzero* value: real
    // std::format enters width-parsing on '0' too and rejects it there
    // ("width must be non-zero"), confirmed by cross-checking GCC's
    // actual consteval diagnostic for "{:00d}" -- excluding '0' here
    // entirely (as an earlier draft did) instead silently skipped width
    // parsing and produced a different, wrong error ("invalid format
    // type") for the same input.
    it = pctx.begin();
    if (it != end && ((*it >= '0' && *it <= '9') || *it == '{')) {
        spec.width = parse_width_or_precision_value(pctx, false);
    }

    it = pctx.begin();
    if (it != end && *it == '.') {
        pctx.advance_to(it + 1);
        spec.precision = parse_width_or_precision_value(pctx, true);
    }

    it = pctx.begin();
    if (it != end && *it == 'L') {
        spec.locale = true;
        ++it;
        pctx.advance_to(it);
    }

    it = pctx.begin();
    if (it != end && *it != '}') {
        if (allowed_types.find(*it) == std::string_view::npos) {
            throw format_error("invalid format type for this argument");
        }
        spec.type = *it;
        ++it;
        pctx.advance_to(it);
    }

    return spec;
}

/// @brief Type-erased access to "argument K, interpreted as an
///        integer" — the only capability dynamic width/precision
///        (`{}`/`{N}` nested inside a format-spec) needs from the
///        full argument list. Deliberately narrower than fully erasing
///        every argument's type (which only `vformat`'s implementation
///        needs, for a different reason): dynamic width/precision is
///        only ever meaningful for integral arguments, so this is all
///        a `format_context` needs to carry to resolve one.
class dynamic_arg_source {
public:
    /// @brief Constructs from a callable `(std::size_t index) -> long long`.
    /// @param fn The callable; throws `format_error` itself for an
    ///        out-of-range index or a non-integral argument.
    explicit dynamic_arg_source(std::function<long long(std::size_t)> fn) : fn_(std::move(fn)) {}

    /// @brief Fetches argument `index` as a `long long`.
    /// @param index The argument index.
    /// @return The argument's value, converted to `long long`.
    /// @throws format_error if `index` is out of range or the
    ///         argument isn't an integral type.
    long long get(std::size_t index) const { return fn_(index); }

private:
    std::function<long long(std::size_t)> fn_;
};

/// @brief Resolves a `width_or_precision` to a concrete value,
///        fetching it from `src` if dynamic.
/// @param wp The parsed width/precision. Precondition: `wp.k !=
///        width_or_precision::kind::none` (callers check this
///        themselves, since "unspecified" and "specified as 0" are
///        different states the caller needs to distinguish).
/// @param src The dynamic-argument source for `wp.k ==
///        width_or_precision::kind::dynamic`.
/// @return The resolved value.
/// @throws format_error if `wp` is dynamic and the referenced argument
///         resolves to a negative value.
inline std::size_t resolve_width_or_precision(const width_or_precision& wp, const dynamic_arg_source& src) {
    if (wp.k == width_or_precision::kind::literal) {
        return wp.value;
    }
    long long v = src.get(wp.value);
    if (v < 0) throw format_error("negative width/precision from a dynamic argument");
    return static_cast<std::size_t>(v);
}

/// @brief The output context passed to `formatter<T>::format`:
///        wherever formatted output goes (`OutIt`), plus access to
///        dynamic width/precision resolution. Matches real
///        `std::basic_format_context<OutIt, char>`'s essential shape
///        — a user's `formatter<T>::format` written against this type
///        is source-compatible with the real one.
/// @tparam OutIt The output iterator type. `format_to<OutIt>` is
///         generic over any output iterator, matching the real
///         signature (not a curated fixed set of sinks) — this is
///         template parameter that makes that possible.
template <class OutIt>
class format_context {
public:
    /// @brief The output iterator type.
    using iterator = OutIt;
    /// @brief The character type. Always `char` (docs/adr/0012).
    using char_type = char;

    /// @brief Constructs a context writing to `out`, with `src` for
    ///        dynamic width/precision resolution.
    /// @param out The output iterator.
    /// @param src The dynamic-argument source. Must outlive this
    ///        context — held by reference, not copied, since it's
    ///        always a short-lived local in whichever top-level
    ///        entry point (`format`/`format_to`/`vformat`) constructed
    ///        this context.
    format_context(OutIt out, const dynamic_arg_source& src) : out_(out), src_(src) {}

    /// @brief The current output iterator.
    /// @return The output iterator.
    iterator out() const { return out_; }
    /// @brief Replaces the output iterator (after writing through a
    ///        local copy of it, e.g. via `std::format_to`).
    /// @param it The new output iterator.
    void advance_to(iterator it) { out_ = it; }

    /// @brief Resolves dynamic width/precision argument `index`.
    /// @param index The argument index.
    /// @return The argument's value as a `long long`.
    /// @throws format_error if `index` is out of range or the
    ///         argument isn't an integral type.
    long long arg_as_dynamic(std::size_t index) const { return src_.get(index); }

private:
    OutIt out_;
    const dynamic_arg_source& src_;
};

/// @brief The `formatter<T>` customization point, disabled by default
///        — matching real `std::formatter`'s "disabled formatter"
///        behavior for any `T` without a specialization. Attempting to
///        use an unformattable type fails to compile with a
///        reasonably clear "deleted function" error rather than a
///        wall of SFINAE errors, via the deleted default constructor.
/// @tparam T The type to format.
template <class T, class = void>
struct formatter {
    /// @brief Deleted: `T` has no `formatter` specialization.
    formatter() = delete;
    /// @brief Deleted: `T` has no `formatter` specialization.
    formatter(const formatter&) = delete;
    /// @brief Deleted: `T` has no `formatter` specialization.
    /// @return Never returns; deleted.
    formatter& operator=(const formatter&) = delete;
};

/// @brief Symbols promoted to `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::format::format_error;
using bridge::detail::truss::cpp17::format::format_parse_context;
using bridge::detail::truss::cpp17::format::format_context;
using bridge::detail::truss::cpp17::format::formatter;
} // namespace exports

} // namespace bridge::detail::truss::cpp17::format

/// @brief Curated re-export surface; see docs/adr/0001-namespace-and-export-scheme.md.
///
/// No `inline namespace format { ... }` wrapper here (same reason as
/// truss/cpp17/expected.hpp's exports): this header's exports will
/// include a function named `format` once the top-level entry points
/// land, and nesting it inside an inline namespace of the identical
/// name makes that inline namespace's own qualified name reachable at
/// this same scope, colliding with the promoted function. Promoting
/// straight from the `cpp17` inline namespace avoids the collision up
/// front rather than hitting it later.
namespace bridge::exports::truss {
inline namespace cpp17 {
using namespace bridge::detail::truss::cpp17::format::exports;
} // namespace cpp17
} // namespace bridge::exports::truss

/// @brief Truss's public API surface.
namespace bridge::truss {
using bridge::exports::truss::format_error;
using bridge::exports::truss::format_parse_context;
using bridge::exports::truss::format_context;
using bridge::exports::truss::formatter;
} // namespace bridge::truss
