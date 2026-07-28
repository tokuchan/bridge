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

`bridge::format`, `bridge::print`, and `bridge::println` match
C++23's `std::format` family. This family has a format-spec
mini-language, a user-extensible `formatter<T>` customization point,
and `print`/`println` built on top.

`format` has no STL facility at all before C++20. So Truss owns the
whole thing outright: the parser, the type-erasure machinery behind
`vformat`, every built-in formatter, and the top-level entry points.
`print` and `println` always call into this same library's own
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
  `L` spec flag parses, but this facility always ignores it. This
  facility behaves as if the `L` flag were absent.
- **No compile-time format-string validation.** C++17 has no
  `consteval` to reject a malformed literal format string at compile
  time. A malformed format string compiles here, and throws
  `format_error` at runtime instead. This page discloses this
  through a `BRIDGE_RIVETS_DIVERGENCE_NOTE`, wherever the polyfill's
  `format_string` is actually selected.
- **Field width is byte-based, not Unicode-display-column-based.**
  This is correct for ASCII content. A computed field comes out
  wider than intended, for multi-byte UTF-8 content.
- **`formatter<T>` cannot transparently unify across the passthrough
  boundary.** `formatter<T>` is the one symbol here users specialize,
  not just name. C++ does not allow specializing an alias template.
  Extending formatting for your own type means specializing
  `bridge::truss::formatter<T>` on the polyfill path, or
  `std::formatter<T>` under passthrough, directly. You cannot
  specialize `bridge::formatter<T>` itself; you can only name it. A
  type meant to stay formattable across a C++17-to-C++20 toolchain
  upgrade genuinely needs both specializations. This is an inherent
  limitation of what alias templates can do in C++, not a gap in
  this facility's design. This page discloses this through its own
  `BRIDGE_RIVETS_DIVERGENCE_NOTE`, on the polyfill branch.

See
[ADR-0012](https://github.com/tokuchan/bridge/blob/master/docs/adr/0012-format-print-truss-owns-the-facility.md)
for the full rationale for all of the above. See
[ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md)
for why the compiler-noted gaps get a `BRIDGE_RIVETS_DIVERGENCE_NOTE`,
rather than only a code comment.

## Passthrough

`std::format` and `std::print`/`std::println` cross their real
passthrough thresholds at different standards: `format` in C++20,
`print`/`println` three years later in C++23. So this facility is
gated by two independent Feature Tests
(`BRIDGE_RIVETS_FEATURES_LIB_FORMAT` and `_LIB_PRINT`), rather than
one. Deck owns two separate alias-selection headers
(`deck/cpp17/format.hpp`, `deck/cpp17/print.hpp`), even though they
are one conceptual division on this page.

A benchmark confirmed that routing `print` through whichever
`format` Deck selected, instead of always Truss's own, would not
cost enough to justify an exception to the "Truss never passes
through" rule (ADR-0012).

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `format` | function | This function formats `args` according to `fmt`, returning the result as a new `std::string`. | [`<format>::format`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_args` | class | This class is a type-erased argument pack for vformat. You construct this class through `make_format_args`. | [`<format>::format_args`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_context` | class | This class is the output context passed to `formatter<T>::format`. This class holds wherever formatted output goes (`OutIt`), plus access to dynamic width/precision resolution. | [`<format>::format_context`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_error` | class | A format string throws this class when it is malformed, references an out-of-range or type-mismatched argument, or otherwise cannot be honored. This class matches `std::format_error`. | [`<format>::format_error`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_parse_context` | class | This class is a cursor over the not-yet-consumed portion of a replacement field's format-spec substring. This class also holds the auto/manual argument-indexing state shared across the whole format string. | [`<format>::format_parse_context`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_string` | class | This class is a format string. This class is implicitly constructible from anything convertible to `std::string_view`. This class matches real `std::format_string<Args...>`'s converting-constructor shape. | [`<format>::format_string`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to` | function | This function formats `args` according to `fmt`, writing to `out`. This function is generic over any output iterator, matching the real signature, not a curated fixed set of sinks. | [`<format>::format_to`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to_n` | function | This function formats `args` according to `fmt`, writing at most `n` characters to `out`. | [`<format>::format_to_n`](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to_n_result` | struct | This struct holds `format_to_n`'s result. `out` is an iterator one past the last element actually written. `size` is the total number of characters that would have been written, for an unlimited output size. This struct matches real `std::format_to_n_result`. | [`<format>::format_to_n_result`](https://en.cppreference.com/w/cpp/utility/format) |
| `formatted_size` | function | This function computes the length `format(fmt, args...)` would produce, without building the string. | [`<format>::formatted_size`](https://en.cppreference.com/w/cpp/utility/format) |
| `formatter` | struct | This struct is the `formatter<T>` customization point. This struct is disabled by default, matching real `std::formatter`'s "disabled formatter" behavior for any `T` without a specialization. | [`<format>::formatter`](https://en.cppreference.com/w/cpp/utility/format) |
| `make_format_args` | function | This function constructs a type-erased `format_args` from `args`, for `vformat`. | [`<format>::make_format_args`](https://en.cppreference.com/w/cpp/utility/format) |
| `print` | function | This function writes `format(fmt, args...)` to `stream`. | [`<format>::print`](https://en.cppreference.com/w/cpp/utility/format) |
| `println` | function | This function writes `format(fmt, args...)` followed by a newline to `stream`. | [`<format>::println`](https://en.cppreference.com/w/cpp/utility/format) |
| `vformat` | function | This function formats a type-erased argument pack according to `fmt`, returning the result as a new `std::string`. Unlike `format`, `fmt` is a plain `std::string_view`. `vformat` is explicitly the entry point for a runtime format string, with no format-string- specific type. This matches real `std::vformat`. | [`<format>::vformat`](https://en.cppreference.com/w/cpp/utility/format) |
<!-- BRIDGE-DOCS:END -->
