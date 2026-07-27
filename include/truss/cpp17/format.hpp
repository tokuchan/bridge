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

#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace bridge::detail::truss::cpp17::format {

/// @brief Thrown when a format string is malformed, references an
///        out-of-range or type-mismatched argument, or otherwise
///        can't be honored. Matches `std::format_error`.
/// @see https://en.cppreference.com/w/cpp/utility/format
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
/// @see https://en.cppreference.com/w/cpp/utility/format
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
/// @see https://en.cppreference.com/w/cpp/utility/format
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
/// @see https://en.cppreference.com/w/cpp/utility/format
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

/// @brief Resolves a `width_or_precision` using a live
///        `format_context`'s dynamic-argument access, for callers that
///        only have a context (not a standalone `dynamic_arg_source`)
///        at hand. See the `dynamic_arg_source` overload for the
///        precondition and error behavior.
/// @tparam OutIt The context's output iterator type.
/// @param wp The parsed width/precision.
/// @param ctx The context to resolve a dynamic reference against.
/// @return The resolved value.
template <class OutIt>
std::size_t resolve_width_or_precision(const width_or_precision& wp, const format_context<OutIt>& ctx) {
    if (wp.k == width_or_precision::kind::literal) return wp.value;
    long long v = ctx.arg_as_dynamic(wp.value);
    if (v < 0) throw format_error("negative width/precision from a dynamic argument");
    return static_cast<std::size_t>(v);
}

/// \cond BRIDGE_DETAIL
///
/// Shared field-writing helpers used by every built-in formatter below
/// -- pure implementation plumbing, never part of the public API.
/// Excluded from the documentation-coverage gate for the same reason
/// as the layered-base machinery in truss/cpp17/expected.hpp: writing
/// full docs for internal helpers with no reader-facing value.
namespace field_writers {

/// @brief Writes `prefix` then `digits`, applying width padding.
///        Honors the "numeric zero-pad" rule: when `spec.zero_pad` is
///        set and no explicit alignment was given, padding zeros go
///        *between* `prefix` and `digits` (so `-0007`, `0x0000ff`, not
///        `000-7`); otherwise padding follows normal fill/align rules
///        with a numeric (right) default alignment. Matches real
///        `std::format`'s behavior for arithmetic types, verified
///        against it directly (not derived from the standard text
///        alone) for every combination this polyfill supports.
template <class OutIt>
void write_numeric_field(format_context<OutIt>& ctx, std::string_view prefix, std::string_view digits,
                          const parsed_std_spec& spec) {
    std::size_t width =
        spec.width.k == width_or_precision::kind::none ? 0 : resolve_width_or_precision(spec.width, ctx);
    std::size_t content_len = prefix.size() + digits.size();
    auto out = ctx.out();
    if (spec.zero_pad && spec.align == align_t::none) {
        for (char c : prefix) *out++ = c;
        if (width > content_len) {
            for (std::size_t i = 0; i < width - content_len; ++i) *out++ = '0';
        }
        for (char c : digits) *out++ = c;
        ctx.advance_to(out);
        return;
    }
    align_t align = spec.align == align_t::none ? align_t::right : spec.align;
    std::size_t pad = width > content_len ? width - content_len : 0;
    std::size_t left = align == align_t::left ? 0 : align == align_t::right ? pad : pad / 2;
    std::size_t right = pad - left;
    for (std::size_t i = 0; i < left; ++i) *out++ = spec.fill;
    for (char c : prefix) *out++ = c;
    for (char c : digits) *out++ = c;
    for (std::size_t i = 0; i < right; ++i) *out++ = spec.fill;
    ctx.advance_to(out);
}

/// @brief Writes `content` (already precision-truncated by the
///        caller, if applicable), applying width padding with a
///        string-like (left) default alignment.
template <class OutIt>
void write_string_field(format_context<OutIt>& ctx, std::string_view content, const parsed_std_spec& spec) {
    std::size_t width =
        spec.width.k == width_or_precision::kind::none ? 0 : resolve_width_or_precision(spec.width, ctx);
    align_t align = spec.align == align_t::none ? align_t::left : spec.align;
    std::size_t pad = width > content.size() ? width - content.size() : 0;
    std::size_t left = align == align_t::left ? 0 : align == align_t::right ? pad : pad / 2;
    std::size_t right = pad - left;
    auto out = ctx.out();
    for (std::size_t i = 0; i < left; ++i) *out++ = spec.fill;
    for (char c : content) *out++ = c;
    for (std::size_t i = 0; i < right; ++i) *out++ = spec.fill;
    ctx.advance_to(out);
}

/// @brief Applies precision-as-max-length truncation to `content`, if
///        `spec` has a precision. Byte-based, not Unicode-display-
///        column-based -- docs/adr/0012's disclosed approximation for
///        multi-byte UTF-8 content.
inline std::string_view truncate_to_precision(std::string_view content, const parsed_std_spec& spec,
                                               std::size_t resolved_precision) {
    if (spec.precision.k == width_or_precision::kind::none) return content;
    return content.substr(0, std::min(content.size(), resolved_precision));
}

/// @brief Debug-format (`?`) escaping for a single character, appended
///        to `out`. Handles `"`, `\`, and the common C escapes
///        (`\n`/`\t`/`\r`); other non-printable bytes become `\xHH`.
///        An ASCII-only approximation of real `std::format`'s
///        Unicode-aware escaping, matching this polyfill's byte-based
///        (not grapheme-based) approach throughout.
inline void append_debug_escaped(std::string& out, char c, char quote) {
    if (c == quote || c == '\\') {
        out += '\\';
        out += c;
    } else if (c == '\n') {
        out += "\\n";
    } else if (c == '\t') {
        out += "\\t";
    } else if (c == '\r') {
        out += "\\r";
    } else if (static_cast<unsigned char>(c) < 0x20 || c == 0x7f) {
        char buf[5];
        std::snprintf(buf, sizeof(buf), "\\x%02x", static_cast<unsigned char>(c));
        out += buf;
    } else {
        out += c;
    }
}

} // namespace field_writers
/// \endcond

/// @brief True for the integer types this polyfill formats generically
///        (matching `std::formatter`'s integer specializations):
///        everything `std::is_integral_v`, except `bool` and `char`
///        (each formatted differently by their own dedicated
///        specializations below) and the wide/Unicode character types
///        (out of scope, docs/adr/0012).
template <class T>
inline constexpr bool is_formattable_integral_v =
    std::is_integral_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char> &&
    !std::is_same_v<T, wchar_t> && !std::is_same_v<T, char16_t> && !std::is_same_v<T, char32_t>;

/// \cond BRIDGE_DETAIL
namespace field_writers {

/// @brief Core integer-formatting logic shared by `formatter<T>` for
///        integer types and the numeric-presentation branches of
///        `formatter<bool>`/`formatter<char>`, so none of the three
///        duplicate it. Not a formatter itself: takes the already-
///        decomposed sign and unsigned magnitude directly, since
///        bool/char's numeric paths don't have a signed value to
///        decompose from in the same way a real integer type does.
template <class OutIt, class U>
void write_integer_value(format_context<OutIt>& ctx, bool negative, U uval, const parsed_std_spec& spec) {
    char type = spec.type ? spec.type : 'd';

    std::string prefix;
    if (negative) {
        prefix += '-';
    } else if (spec.sign == sign_t::plus) {
        prefix += '+';
    } else if (spec.sign == sign_t::space) {
        prefix += ' ';
    }

    int base = 10;
    switch (type) {
    case 'b':
        base = 2;
        if (spec.alt) prefix += "0b";
        break;
    case 'B':
        base = 2;
        if (spec.alt) prefix += "0B";
        break;
    case 'o':
        base = 8;
        if (spec.alt && uval != 0) prefix += "0";
        break;
    case 'x':
        base = 16;
        if (spec.alt) prefix += "0x";
        break;
    case 'X':
        base = 16;
        if (spec.alt) prefix += "0X";
        break;
    default:
        base = 10;
        break;
    }

    char buf[80];
    auto res = std::to_chars(buf, buf + sizeof(buf), uval, base);
    std::string digits(buf, res.ptr);
    if (type == 'X') {
        for (auto& c : digits) {
            if (c >= 'a' && c <= 'z') c = static_cast<char>(c - ('a' - 'A'));
        }
    }

    write_numeric_field(ctx, prefix, digits, spec);
}

} // namespace field_writers
/// \endcond

/// @brief `formatter<T>` for integer types: presentation types `d`
///        (default), `b`/`B`, `o`, `x`/`X`. Precision isn't valid.
///
///        Matches real `std::formatter`'s integer specializations,
///        including precision's rejection (confirmed by hitting its
///        actual compile-time rejection of `"{:.2}"` against an `int`
///        argument, not assumed).
template <class T>
struct formatter<T, std::enable_if_t<is_formattable_integral_v<T>>> {
    /// @brief Parses the format-spec.
    /// @param pctx The parse context.
    /// @return An iterator to the field's closing `}`.
    format_parse_context::iterator parse(format_parse_context& pctx) {
        spec_ = parse_std_spec(pctx, "dbBoxX");
        if (spec_.precision.k != width_or_precision::kind::none) {
            throw format_error("precision is not valid for integer types");
        }
        return pctx.begin();
    }

    /// @brief Formats `value` into `ctx`.
    /// @tparam FormatContext The context type (any `format_context<OutIt>`).
    /// @param value The value to format.
    /// @param ctx The output context.
    /// @return `ctx.out()` after writing.
    template <class FormatContext>
    auto format(T value, FormatContext& ctx) const -> typename FormatContext::iterator {
        using U = std::make_unsigned_t<T>;
        bool negative = false;
        U uval;
        if constexpr (std::is_signed_v<T>) {
            negative = value < 0;
            uval = negative ? static_cast<U>(0) - static_cast<U>(value) : static_cast<U>(value);
        } else {
            uval = value;
        }
        field_writers::write_integer_value(ctx, negative, uval, spec_);
        return ctx.out();
    }

private:
    parsed_std_spec spec_{};
};

/// @brief `formatter<bool>`: default/`s` presentation writes `"true"`/
///        `"false"` (string-like, left-aligned by default); numeric
///        presentations (`d`/`b`/`B`/`o`/`x`/`X`) treat it as `0`/`1`.
///        Precision isn't valid.
///
///        Matches real `std::format` (confirmed by hitting its actual
///        compile-time rejection of precision, not assumed).
template <>
struct formatter<bool> {
    /// @brief Parses the format-spec.
    /// @param pctx The parse context.
    /// @return An iterator to the field's closing `}`.
    format_parse_context::iterator parse(format_parse_context& pctx) {
        spec_ = parse_std_spec(pctx, "sdbBoxX");
        if (spec_.precision.k != width_or_precision::kind::none) {
            throw format_error("precision is not valid for bool");
        }
        return pctx.begin();
    }

    /// @brief Formats `value` into `ctx`.
    /// @tparam FormatContext The context type (any `format_context<OutIt>`).
    /// @param value The value to format.
    /// @param ctx The output context.
    /// @return `ctx.out()` after writing.
    template <class FormatContext>
    auto format(bool value, FormatContext& ctx) const -> typename FormatContext::iterator {
        if (spec_.type == '\0' || spec_.type == 's') {
            field_writers::write_string_field(ctx, value ? "true" : "false", spec_);
            return ctx.out();
        }
        field_writers::write_integer_value(ctx, false, static_cast<unsigned>(value ? 1 : 0), spec_);
        return ctx.out();
    }

private:
    parsed_std_spec spec_{};
};

/// @brief `formatter<char>`: default/`c` presentation writes the
///        character itself (string-like, left-aligned by default);
///        numeric presentations (`d`/`b`/`B`/`o`/`x`/`X`) treat it as
///        its underlying integer value; `?` (C++23 debug format) wraps
///        it in single quotes with the same escaping
///        `formatter<std::string>` uses -- confirmed real
///        `std::format` supports `{:?}` for `char` too (e.g. `'x'`,
///        `'\n'`), not just strings, before adding it here. Precision
///        isn't valid.
template <>
struct formatter<char> {
    /// @brief Parses the format-spec.
    /// @param pctx The parse context.
    /// @return An iterator to the field's closing `}`.
    format_parse_context::iterator parse(format_parse_context& pctx) {
        spec_ = parse_std_spec(pctx, "c?dbBoxX");
        if (spec_.precision.k != width_or_precision::kind::none) {
            throw format_error("precision is not valid for char");
        }
        return pctx.begin();
    }

    /// @brief Formats `value` into `ctx`.
    /// @tparam FormatContext The context type (any `format_context<OutIt>`).
    /// @param value The value to format.
    /// @param ctx The output context.
    /// @return `ctx.out()` after writing.
    template <class FormatContext>
    auto format(char value, FormatContext& ctx) const -> typename FormatContext::iterator {
        if (spec_.type == '?') {
            std::string escaped;
            escaped += '\'';
            field_writers::append_debug_escaped(escaped, value, '\'');
            escaped += '\'';
            field_writers::write_string_field(ctx, escaped, spec_);
            return ctx.out();
        }
        if (spec_.type == '\0' || spec_.type == 'c') {
            field_writers::write_string_field(ctx, std::string_view(&value, 1), spec_);
            return ctx.out();
        }
        field_writers::write_integer_value(ctx, false, static_cast<unsigned char>(value), spec_);
        return ctx.out();
    }

private:
    parsed_std_spec spec_{};
};

/// @brief True for the floating-point types this polyfill formats
///        (`float`, `double`, `long double`).
template <class T>
inline constexpr bool is_formattable_float_v = std::is_floating_point_v<T>;

/// @brief `formatter<T>` for floating-point types: presentation types
///        `f`/`F` (fixed), `e`/`E` (scientific), `g`/`G` (general),
///        `a`/`A` (hex), or none (shortest round-trip when no
///        precision is given either, matching `std::to_chars`'
///        default; otherwise behaves like `g`). Built on
///        `std::to_chars` (a real C++17 facility) for the actual
///        numeric conversion rather than a hand-rolled algorithm --
///        `f`/`F`/`e`/`E`/`g`/`G` default to precision 6 when no
///        precision is given, matching `printf`'s convention; this was
///        confirmed against real `std::format`'s actual output, not
///        assumed from the standard text alone (it doesn't use
///        `to_chars`' own shortest-round-trip default the way the
///        type-less presentation does).
template <class T>
struct formatter<T, std::enable_if_t<is_formattable_float_v<T>>> {
    /// @brief Parses the format-spec.
    /// @param pctx The parse context.
    /// @return An iterator to the field's closing `}`.
    format_parse_context::iterator parse(format_parse_context& pctx) {
        spec_ = parse_std_spec(pctx, "fFeEgGaA");
        return pctx.begin();
    }

    /// @brief Formats `value` into `ctx`.
    /// @tparam FormatContext The context type (any `format_context<OutIt>`).
    /// @param value The value to format.
    /// @param ctx The output context.
    /// @return `ctx.out()` after writing.
    template <class FormatContext>
    auto format(T value, FormatContext& ctx) const -> typename FormatContext::iterator {
        char type = spec_.type;
        bool negative = std::signbit(value);
        T abs_value = negative ? -value : value;

        std::string prefix;
        if (negative) {
            prefix += '-';
        } else if (spec_.sign == sign_t::plus) {
            prefix += '+';
        } else if (spec_.sign == sign_t::space) {
            prefix += ' ';
        }

        bool uppercase = false;
        std::chars_format cf = std::chars_format::general;
        switch (type) {
        case 'f':
            cf = std::chars_format::fixed;
            break;
        case 'F':
            cf = std::chars_format::fixed;
            uppercase = true;
            break;
        case 'e':
            cf = std::chars_format::scientific;
            break;
        case 'E':
            cf = std::chars_format::scientific;
            uppercase = true;
            break;
        case 'g':
            cf = std::chars_format::general;
            break;
        case 'G':
            cf = std::chars_format::general;
            uppercase = true;
            break;
        case 'a':
            cf = std::chars_format::hex;
            break;
        case 'A':
            cf = std::chars_format::hex;
            uppercase = true;
            break;
        default:
            break;
        }

        bool has_precision = spec_.precision.k != width_or_precision::kind::none;
        std::size_t precision = has_precision ? resolve_width_or_precision(spec_.precision, ctx) : 0;

        char buf[512];
        std::to_chars_result res{};
        if (type == '\0' && !has_precision) {
            res = std::to_chars(buf, buf + sizeof(buf), abs_value);
        } else if ((type == 'a' || type == 'A') && !has_precision) {
            res = std::to_chars(buf, buf + sizeof(buf), abs_value, cf);
        } else {
            if (!has_precision) precision = 6;
            res = std::to_chars(buf, buf + sizeof(buf), abs_value, cf, static_cast<int>(precision));
        }
        std::string digits(buf, res.ptr);
        if (uppercase) {
            for (auto& c : digits) {
                if (c >= 'a' && c <= 'z') c = static_cast<char>(c - ('a' - 'A'));
            }
        }

        field_writers::write_numeric_field(ctx, prefix, digits, spec_);
        return ctx.out();
    }

private:
    parsed_std_spec spec_{};
};

/// @brief `formatter<T>` for pointer types (`void*`, `const void*`,
///        `std::nullptr_t`): presentation `p` (default and only valid
///        type) writes `0x` followed by the address in lowercase hex,
///        no leading zeros, right-aligned by default. Precision isn't
///        valid.
///
///        Confirmed against real `std::format`'s actual output
///        (including that `nullptr` formats as `"0x0"`, not
///        zero-padded to pointer width), not assumed.
template <class T>
struct formatter<T, std::enable_if_t<std::is_same_v<T, const void*> || std::is_same_v<T, void*> ||
                                      std::is_same_v<T, std::nullptr_t>>> {
    /// @brief Parses the format-spec.
    /// @param pctx The parse context.
    /// @return An iterator to the field's closing `}`.
    format_parse_context::iterator parse(format_parse_context& pctx) {
        spec_ = parse_std_spec(pctx, "p");
        if (spec_.precision.k != width_or_precision::kind::none) {
            throw format_error("precision is not valid for pointers");
        }
        return pctx.begin();
    }

    /// @brief Formats `value` into `ctx`.
    /// @tparam FormatContext The context type (any `format_context<OutIt>`).
    /// @param value The value to format.
    /// @param ctx The output context.
    /// @return `ctx.out()` after writing.
    template <class FormatContext>
    auto format(T value, FormatContext& ctx) const -> typename FormatContext::iterator {
        std::uintptr_t addr;
        if constexpr (std::is_same_v<T, std::nullptr_t>) {
            addr = 0;
        } else {
            addr = reinterpret_cast<std::uintptr_t>(value);
        }
        char buf[32];
        auto res = std::to_chars(buf, buf + sizeof(buf), addr, 16);
        std::string digits(buf, res.ptr);
        field_writers::write_numeric_field(ctx, "0x", digits, spec_);
        return ctx.out();
    }

private:
    parsed_std_spec spec_{};
};

/// @brief `formatter<T>` for the string-like types (`const char*`,
///        `char*`, `std::string`, `std::string_view`): presentation
///        `s` (default) writes the content as-is; `?` (C++23 debug
///        format) wraps it in quotes with `\"`/`\\`/`\n`/`\t`/`\r`
///        escaping (and `\xHH` for other non-printable bytes) -- an
///        ASCII-only approximation of real `std::format`'s
///        Unicode-aware escaping, matching this polyfill's byte-based
///        approach throughout (docs/adr/0012). Precision truncates to
///        at most that many bytes (also byte-based, not
///        display-column-based).
template <class T>
struct formatter<T, std::enable_if_t<std::is_same_v<T, const char*> || std::is_same_v<T, char*> ||
                                      std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>>> {
    /// @brief Parses the format-spec.
    /// @param pctx The parse context.
    /// @return An iterator to the field's closing `}`.
    format_parse_context::iterator parse(format_parse_context& pctx) {
        spec_ = parse_std_spec(pctx, "s?");
        return pctx.begin();
    }

    /// @brief Formats `value` into `ctx`.
    /// @tparam FormatContext The context type (any `format_context<OutIt>`).
    /// @param value The value to format.
    /// @param ctx The output context.
    /// @return `ctx.out()` after writing.
    template <class FormatContext>
    auto format(const T& value, FormatContext& ctx) const -> typename FormatContext::iterator {
        std::string_view sv(value);
        if (spec_.type == '?') {
            std::string escaped;
            escaped += '"';
            for (char c : sv) field_writers::append_debug_escaped(escaped, c, '"');
            escaped += '"';
            std::size_t precision = spec_.precision.k == width_or_precision::kind::none
                                         ? escaped.size()
                                         : resolve_width_or_precision(spec_.precision, ctx);
            field_writers::write_string_field(
                ctx, field_writers::truncate_to_precision(escaped, spec_, precision), spec_);
            return ctx.out();
        }
        std::size_t precision = spec_.precision.k == width_or_precision::kind::none
                                     ? sv.size()
                                     : resolve_width_or_precision(spec_.precision, ctx);
        field_writers::write_string_field(ctx, field_writers::truncate_to_precision(sv, spec_, precision), spec_);
        return ctx.out();
    }

private:
    parsed_std_spec spec_{};
};

/// @brief A format string, implicitly constructible from anything
///        convertible to `std::string_view` (matching real
///        `std::format_string<Args...>`'s converting-constructor
///        shape). Unlike the real type, this constructor does **not**
///        validate the format string at compile time -- C++17 has no
///        `consteval` to do that with (docs/adr/0012's disclosed
///        compile-time-validation gap). Validation happens only when
///        the string is actually used to format, at which point a
///        malformed spec throws `format_error` same as `vformat`'s
///        contract always has.
/// @tparam Args The argument types this format string is meant to be
///         used with (unused by this type itself; carried only so
///         `format`/`format_to`/etc.'s signatures match the real ones).
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class... Args>
class format_string {
public:
    /// @brief Converts from anything convertible to `std::string_view`.
    /// @tparam T The source type.
    /// @param s The format string.
    template <class T, class = std::enable_if_t<std::is_convertible_v<const T&, std::string_view>>>
    constexpr format_string(const T& s) noexcept : fmt_(s) {}

    /// @brief The underlying format string.
    /// @return The format string as a `std::string_view`.
    constexpr std::string_view get() const noexcept { return fmt_; }

private:
    std::string_view fmt_;
};

/// \cond BRIDGE_DETAIL
///
/// The parsing/dispatch engine shared by every top-level entry point
/// below, plus the runtime-index-into-compile-time-argument-pack
/// dispatch `format_to`/`format`/`format_to_n`/`formatted_size` use,
/// and the type-erased equivalent `vformat`/`format_args` use. Pure
/// implementation plumbing, excluded from the documentation-coverage
/// gate for the same reason as `field_writers` above.
namespace engine {

template <class T>
long long as_dynamic_value(const T& v) {
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool>) {
        return static_cast<long long>(v);
    } else {
        throw format_error("dynamic width/precision argument is not an integer");
    }
}

template <class Tuple, std::size_t... I>
long long get_dynamic_impl(std::size_t index, Tuple& t, std::index_sequence<I...>) {
    long long result = 0;
    bool handled = ((index == I && (result = as_dynamic_value(std::get<I>(t)), true)) || ...);
    if (!handled) throw format_error("dynamic width/precision argument index out of range");
    return result;
}

template <class FormatContext, class T>
void format_one(format_parse_context& pctx, FormatContext& ctx, T&& value) {
    using U = std::decay_t<T>;
    formatter<U> f;
    auto it = f.parse(pctx);
    pctx.advance_to(it);
    f.format(value, ctx);
}

template <class FormatContext, class Tuple, std::size_t... I>
void dispatch_impl(std::size_t index, format_parse_context& pctx, FormatContext& ctx, Tuple& t,
                    std::index_sequence<I...>) {
    bool handled = ((index == I && (format_one(pctx, ctx, std::get<I>(t)), true)) || ...);
    if (!handled) throw format_error("argument index out of range");
}

/// @brief Walks `fmt`, copying literal text (handling `{{`/`}}`
///        escapes) and dispatching each replacement field to
///        `dispatch`. `dispatch(index, pctx, ctx)` is responsible for
///        calling that argument's `formatter<T>::parse`/`format` and
///        advancing `pctx` past the spec it consumed; this function
///        handles everything else (arg-id parsing/auto-vs-manual
///        indexing, locating and consuming the field's closing `}`,
///        literal-text copying).
template <class OutIt, class Dispatch>
OutIt format_engine(std::string_view fmt, OutIt out, const dynamic_arg_source& src, std::size_t num_args,
                     Dispatch&& dispatch) {
    format_parse_context pctx(fmt, num_args);
    format_context<OutIt> ctx(out, src);
    auto it = pctx.begin();
    auto end = pctx.end();
    while (it != end) {
        char c = *it;
        if (c == '{') {
            ++it;
            if (it != end && *it == '{') {
                auto o = ctx.out();
                *o++ = '{';
                ctx.advance_to(o);
                ++it;
                pctx.advance_to(it);
                continue;
            }
            std::size_t index;
            if (it != end && *it >= '0' && *it <= '9') {
                std::size_t id = 0;
                while (it != end && *it >= '0' && *it <= '9') {
                    id = id * 10 + static_cast<std::size_t>(*it - '0');
                    ++it;
                }
                pctx.advance_to(it);
                pctx.check_arg_id(id);
                index = id;
            } else {
                pctx.advance_to(it);
                index = pctx.next_arg_id();
                it = pctx.begin();
            }
            if (it != end && *it == ':') {
                ++it;
                pctx.advance_to(it);
            }
            dispatch(index, pctx, ctx);
            it = pctx.begin();
            if (it == end || *it != '}') throw format_error("expected '}' in format string");
            ++it;
            pctx.advance_to(it);
        } else if (c == '}') {
            ++it;
            if (it != end && *it == '}') {
                auto o = ctx.out();
                *o++ = '}';
                ctx.advance_to(o);
                ++it;
                pctx.advance_to(it);
                continue;
            }
            throw format_error("unmatched '}' in format string");
        } else {
            auto start = it;
            while (it != end && *it != '{' && *it != '}') ++it;
            auto o = ctx.out();
            for (auto p = start; p != it; ++p) *o++ = *p;
            ctx.advance_to(o);
            pctx.advance_to(it);
        }
    }
    return ctx.out();
}

/// @brief Output iterator that counts every assignment (whether or not
///        it writes through) and only writes through to `out_` while
///        under `max_count_` -- the mechanism `format_to_n` uses to
///        report the full untruncated size while only actually writing
///        up to `n` characters.
template <class OutIt>
class counted_output_iterator {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    counted_output_iterator(OutIt out, std::ptrdiff_t max_count) : out_(out), max_count_(max_count) {}
    counted_output_iterator& operator*() { return *this; }
    counted_output_iterator& operator++() { return *this; }
    counted_output_iterator& operator++(int) { return *this; }
    counted_output_iterator& operator=(char c) {
        if (count_ < max_count_) {
            *out_ = c;
            ++out_;
        }
        ++count_;
        return *this;
    }

    std::ptrdiff_t count() const noexcept { return count_; }
    OutIt base() const { return out_; }

private:
    OutIt out_;
    std::ptrdiff_t max_count_;
    std::ptrdiff_t count_ = 0;
};

/// @brief Output iterator that only counts, for `formatted_size`.
class counting_output_iterator {
public:
    using iterator_category = std::output_iterator_tag;
    using value_type = void;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = void;

    counting_output_iterator& operator*() { return *this; }
    counting_output_iterator& operator++() { return *this; }
    counting_output_iterator& operator++(int) { return *this; }
    counting_output_iterator& operator=(char) {
        ++count_;
        return *this;
    }

    std::size_t count() const noexcept { return count_; }

private:
    std::size_t count_ = 0;
};

} // namespace engine
/// \endcond

/// @brief The result of `format_to_n`: `out` is an iterator one past
///        the last element actually written; `size` is the total
///        number of characters that *would* have been written for an
///        unlimited output size, matching real `std::format_to_n_result`.
/// @tparam OutIt The output iterator type.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class OutIt>
struct format_to_n_result {
    /// @brief One past the last element actually written.
    OutIt out;
    /// @brief The total number of characters that would have been
    ///        written for an unlimited output size.
    std::ptrdiff_t size;
};

/// @brief Formats `args` according to `fmt`, writing to `out`. Generic
///        over any output iterator, matching the real signature (not
///        a curated fixed set of sinks).
/// @tparam OutIt The output iterator type.
/// @tparam Args The argument types.
/// @param out The output iterator.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @return An iterator one past the last character written.
/// @throws format_error if `fmt` is malformed or references an
///         out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class OutIt, class... Args>
OutIt format_to(OutIt out, format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    auto args_tuple = std::forward_as_tuple(std::forward<Args>(args)...);
    dynamic_arg_source src([&args_tuple](std::size_t index) -> long long {
        return engine::get_dynamic_impl(index, args_tuple, std::index_sequence_for<Args...>{});
    });
    return engine::format_engine(fmt.get(), out, src, sizeof...(Args),
                                  [&](std::size_t index, format_parse_context& pctx, auto& ctx) {
                                      engine::dispatch_impl(index, pctx, ctx, args_tuple,
                                                             std::index_sequence_for<Args...>{});
                                  });
}

/// @brief Formats `args` according to `fmt`, returning the result as a
///        new `std::string`.
/// @tparam Args The argument types.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @return The formatted string.
/// @throws format_error if `fmt` is malformed or references an
///         out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class... Args>
std::string format(format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    std::string result;
    format_to(std::back_inserter(result), fmt, std::forward<Args>(args)...);
    return result;
}

/// @brief Formats `args` according to `fmt`, writing at most `n`
///        characters to `out`.
/// @tparam OutIt The output iterator type.
/// @tparam Args The argument types.
/// @param out The output iterator.
/// @param n The maximum number of characters to write.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @return The iterator one past the last element actually written,
///         and the total (untruncated) size, per `format_to_n_result`.
/// @throws format_error if `fmt` is malformed or references an
///         out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class OutIt, class... Args>
format_to_n_result<OutIt> format_to_n(OutIt out, std::ptrdiff_t n, format_string<std::decay_t<Args>...> fmt,
                                       Args&&... args) {
    engine::counted_output_iterator<OutIt> counted(out, n);
    auto result = format_to(counted, fmt, std::forward<Args>(args)...);
    return format_to_n_result<OutIt>{result.base(), result.count()};
}

/// @brief Computes the length `format(fmt, args...)` would produce,
///        without building the string.
/// @tparam Args The argument types.
/// @param fmt The format string.
/// @param args The arguments to format.
/// @return The length, in characters.
/// @throws format_error if `fmt` is malformed or references an
///         out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class... Args>
std::size_t formatted_size(format_string<std::decay_t<Args>...> fmt, Args&&... args) {
    engine::counting_output_iterator counter;
    auto result = format_to(counter, fmt, std::forward<Args>(args)...);
    return result.count();
}

/// @brief Type-erased argument pack for @ref vformat, constructed via
///        `make_format_args`. Narrower in scope than real
///        `std::format_args`: pinned to `format_context<
///        std::back_insert_iterator<std::string>>` rather than generic
///        over any output iterator, since this polyfill's scope
///        doesn't include `vformat_to` (only plain `vformat`, which
///        always targets `std::string`) -- a direct, disclosed
///        consequence of docs/adr/0012's scope decision, not a
///        separate divergence in its own right.
/// @see https://en.cppreference.com/w/cpp/utility/format
class format_args {
public:
    /// @brief Dispatches to argument `index`'s `formatter<T>::parse`/
    ///        `format`, matching the compile-time-pack dispatch
    ///        `format_to` uses but through this type-erased path.
    /// @param index The argument index.
    /// @param pctx The parse context, positioned at the field's spec.
    /// @param ctx The output context.
    void format_arg(std::size_t index, format_parse_context& pctx,
                     format_context<std::back_insert_iterator<std::string>>& ctx) const {
        format_fn_(index, pctx, ctx);
    }

    /// @brief Resolves dynamic width/precision argument `index`.
    /// @param index The argument index.
    /// @return The argument's value as a `long long`.
    long long dynamic_arg(std::size_t index) const { return dynamic_fn_(index); }

    /// @brief The number of arguments.
    /// @return The argument count.
    std::size_t count() const noexcept { return count_; }

private:
    template <class... Args>
    friend format_args make_format_args(Args&... args);

    std::function<void(std::size_t, format_parse_context&, format_context<std::back_insert_iterator<std::string>>&)>
        format_fn_;
    std::function<long long(std::size_t)> dynamic_fn_;
    std::size_t count_ = 0;
};

/// @brief Constructs a type-erased `format_args` from `args`, for
///        `vformat`. Matches real `std::make_format_args`' shape
///        (lvalue references -- called with the named parameters of
///        whatever function is forwarding into `vformat`, which are
///        themselves lvalues regardless of the original argument's
///        value category). The result is only valid for the duration
///        of the full expression that calls `vformat` with it, same
///        lifetime constraint as the real function.
/// @tparam Args The argument types.
/// @param args The arguments.
/// @return The type-erased argument pack.
/// @see https://en.cppreference.com/w/cpp/utility/format
template <class... Args>
format_args make_format_args(Args&... args) {
    format_args fa;
    fa.count_ = sizeof...(Args);
    fa.format_fn_ = [&args...](std::size_t index, format_parse_context& pctx,
                                format_context<std::back_insert_iterator<std::string>>& ctx) {
        auto t = std::forward_as_tuple(args...);
        engine::dispatch_impl(index, pctx, ctx, t, std::index_sequence_for<Args...>{});
    };
    fa.dynamic_fn_ = [&args...](std::size_t index) -> long long {
        auto t = std::forward_as_tuple(args...);
        return engine::get_dynamic_impl(index, t, std::index_sequence_for<Args...>{});
    };
    return fa;
}

/// @brief Formats a type-erased argument pack according to `fmt`,
///        returning the result as a new `std::string`. Unlike
///        `format`, `fmt` is a plain `std::string_view` -- `vformat`
///        is explicitly the "runtime format string, no format-string-
///        specific type" entry point, matching real `std::vformat`.
/// @param fmt The format string.
/// @param args The type-erased arguments, from `make_format_args`.
/// @return The formatted string.
/// @throws format_error if `fmt` is malformed or references an
///         out-of-range or type-mismatched argument.
/// @see https://en.cppreference.com/w/cpp/utility/format
inline std::string vformat(std::string_view fmt, format_args args) {
    std::string result;
    dynamic_arg_source src([&args](std::size_t index) -> long long { return args.dynamic_arg(index); });
    engine::format_engine(fmt, std::back_inserter(result), src, args.count(),
                           [&](std::size_t index, format_parse_context& pctx, auto& ctx) {
                               args.format_arg(index, pctx, ctx);
                           });
    return result;
}

/// @brief Symbols promoted to `bridge::exports::truss`.
namespace exports {
using bridge::detail::truss::cpp17::format::format_error;
using bridge::detail::truss::cpp17::format::format_parse_context;
using bridge::detail::truss::cpp17::format::format_context;
using bridge::detail::truss::cpp17::format::formatter;
using bridge::detail::truss::cpp17::format::format_string;
using bridge::detail::truss::cpp17::format::format_to_n_result;
using bridge::detail::truss::cpp17::format::format_to;
using bridge::detail::truss::cpp17::format::format;
using bridge::detail::truss::cpp17::format::format_to_n;
using bridge::detail::truss::cpp17::format::formatted_size;
using bridge::detail::truss::cpp17::format::format_args;
using bridge::detail::truss::cpp17::format::make_format_args;
using bridge::detail::truss::cpp17::format::vformat;
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
using bridge::exports::truss::format_string;
using bridge::exports::truss::format_to_n_result;
using bridge::exports::truss::format_to;
using bridge::exports::truss::format;
using bridge::exports::truss::format_to_n;
using bridge::exports::truss::formatted_size;
using bridge::exports::truss::format_args;
using bridge::exports::truss::make_format_args;
using bridge::exports::truss::vformat;
} // namespace bridge::truss
