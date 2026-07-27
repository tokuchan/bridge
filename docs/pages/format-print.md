\page page_format_print Format Print

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<format>`](https://en.cppreference.com/w/cpp/utility/format) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/format.hpp`
- `include/truss/cpp17/print.hpp`
- `include/deck/cpp17/format.hpp`
- `include/deck/cpp17/print.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::format`/`bridge::print`/`bridge::println` match C++23's
`std::format` family: a format-spec mini-language, a user-extensible
`formatter<T>` customization point, and `print`/`println` built on top.
`format` has no pre-existing STL facility at all before C++20, so Truss
owns the whole thing outright -- the parser, the type-erasure machinery
behind `vformat`, every built-in formatter, and the top-level entry
points -- and `print`/`println` always call into *this same library's*
`format`, never whichever engine Deck happened to select.

## Example

```cpp
#include <deck/cpp17/format.hpp>
#include <deck/cpp17/print.hpp>
#include <string>
#include <cassert>

int main() {
    std::string s = bridge::format("{} is {:.1f}% done", "build", 87.5);
    assert(s == "build is 87.5% done");
    bridge::println("{}", s);
}
```

## Divergences

- **No range or chrono formatting, no locale-aware formatting.** The
  `L` spec flag parses but is always ignored, behaving as if absent.
- **No compile-time format-string validation.** C++17 has no
  `consteval` to reject a malformed literal format string at compile
  time; it compiles here and throws `format_error` at runtime instead
  -- disclosed via a `BRIDGE_RIVETS_DIVERGENCE_NOTE` wherever the
  polyfill's `format_string` is actually selected.
- **Field width is byte-based, not Unicode-display-column-based.**
  Correct for ASCII content; a computed field comes out wider than
  intended for multi-byte UTF-8 content.
- **`formatter<T>` can't transparently unify across the passthrough
  boundary.** It's the one symbol here users *specialize* rather than
  just name, and C++ doesn't allow specializing an alias template.
  Extending formatting for your own type means specializing
  `bridge::truss::formatter<T>` (polyfill path) or `std::formatter<T>`
  (passthrough) directly -- not `bridge::formatter<T>` itself, which
  can be named but never specialized. A type meant to stay formattable
  across a C++17-&gt;C++20 toolchain upgrade genuinely needs both
  specializations; this is an inherent limitation of what alias
  templates can do in C++, not a gap in this facility's design.
  Disclosed via its own `BRIDGE_RIVETS_DIVERGENCE_NOTE` on the polyfill
  branch.

Full rationale for all of the above in
[ADR-0012](https://github.com/tokuchan/bridge/blob/master/docs/adr/0012-format-print-truss-owns-the-facility.md);
why the compiler-noted gaps get a `BRIDGE_RIVETS_DIVERGENCE_NOTE` rather
than only a code comment in
[ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md).

## Passthrough

`std::format` and `std::print`/`std::println` cross their real
passthrough thresholds at *different* standards -- `format` in C++20,
`print`/`println` three years later in C++23 -- so this facility is
gated by two independent Feature Tests
(`BRIDGE_RIVETS_FEATURES_LIB_FORMAT`/`_LIB_PRINT`) rather than one, and
Deck owns two separate alias-selection headers
(`deck/cpp17/format.hpp`, `deck/cpp17/print.hpp`) even though they're
one conceptual division on this page. Confirmed by benchmark that
routing `print` through whichever `format` Deck selected (instead of
always Truss's own) wouldn't cost enough to justify an exception to
the "Truss never passes through" rule (ADR-0012).

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `format` | function | Formats `args` according to `fmt`, returning the result as a new `std::string`. | [`<format>::format`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_args` | class | Type-erased argument pack for vformat, constructed via `make_format_args`. | [`<format>::format_args`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_context` | class | The output context passed to `formatter<T>::format`: wherever formatted output goes (`OutIt`), plus access to dynamic width/precision resolution. Matches real `std::basic_format_context<OutIt, char>`'s essential shape — a user's `formatter<T>::format` written against this type is source-compatible with the real one. | [`<format>::format_context`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_error` | class | Thrown when a format string is malformed, references an out-of-range or type-mismatched argument, or otherwise can't be honored. Matches `std::format_error`. | [`<format>::format_error`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_parse_context` | class | Cursor over the not-yet-consumed portion of a replacement field's format-spec substring, plus the auto/manual argument-indexing state shared across the whole format string. | [`<format>::format_parse_context`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_string` | class | A format string, implicitly constructible from anything convertible to `std::string_view`, matching real `std::format_string<Args...>`'s converting-constructor shape. | [`<format>::format_string`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to` | function | Formats `args` according to `fmt`, writing to `out`. Generic over any output iterator, matching the real signature (not a curated fixed set of sinks). | [`<format>::format_to`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to_n` | function | Formats `args` according to `fmt`, writing at most `n` characters to `out`. | [`<format>::format_to_n`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to_n_result` | struct | The result of `format_to_n`: `out` is an iterator one past the last element actually written; `size` is the total number of characters that *would* have been written for an unlimited output size, matching real `std::format_to_n_result`. | [`<format>::format_to_n_result`](https://en.cppreference.com/w/cpp/utility/format) |
| `formatted_size` | function | Computes the length `format(fmt, args...)` would produce, without building the string. | [`<format>::formatted_size`](https://en.cppreference.com/w/cpp/utility/format) |
| `formatter` | struct | The `formatter<T>` customization point, disabled by default — matching real `std::formatter`'s "disabled formatter" behavior for any `T` without a specialization. Attempting to use an unformattable type fails to compile with a reasonably clear "deleted function" error rather than a wall of SFINAE errors, via the deleted default constructor. | [`<format>::formatter`](https://en.cppreference.com/w/cpp/utility/format) |
| `make_format_args` | function | Constructs a type-erased `format_args` from `args`, for `vformat`. | [`<format>::make_format_args`](https://en.cppreference.com/w/cpp/utility/format) |
| `print` | function | Writes `format(fmt, args...)` to `stream`. | [`<format>::print`](https://en.cppreference.com/w/cpp/utility/format) |
| `println` | function | Writes `format(fmt, args...)` followed by a newline to `stream`. | [`<format>::println`](https://en.cppreference.com/w/cpp/utility/format) |
| `vformat` | function | Formats a type-erased argument pack according to `fmt`, returning the result as a new `std::string`. Unlike `format`, `fmt` is a plain `std::string_view` -- `vformat` is explicitly the "runtime format string, no format-string- specific type" entry point, matching real `std::vformat`. | [`<format>::vformat`](https://en.cppreference.com/w/cpp/utility/format) |
<!-- BRIDGE-DOCS:END -->
