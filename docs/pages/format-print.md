\page page_format_print Format Print

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/format.hpp`
- `include/truss/cpp17/print.hpp`
- `include/deck/cpp17/format.hpp`
- `include/deck/cpp17/print.hpp`
<!-- BRIDGE-DOCS:END -->

`std::format` and `std::print`/`std::println` cross their real
passthrough thresholds at *different* standards -- `format` in C++20,
`print`/`println` three years later in C++23 -- so this facility is
gated by two independent Feature Tests
(`BRIDGE_RIVETS_FEATURES_LIB_FORMAT`/`_LIB_PRINT`) rather than one, and
Deck owns two separate alias-selection headers
(`deck/cpp17/format.hpp`, `deck/cpp17/print.hpp`) even though they're
one conceptual division on this page. `format` itself is function-shaped
with no pre-existing STL facility at all before C++20 -- unlike
`optional` (free functions on an existing type) or `expected` (a class,
since no existing type to extend) -- so Truss owns the whole thing: a
hand-written format-spec parser, the type-erasure machinery behind
`vformat`, the full two-phase `formatter<T>` customization point, every
built-in formatter, and the top-level entry points. `print`/`println`
are built on top, always calling into *this same library's* `format`
-- never whichever `format` Deck happened to select -- confirmed by
benchmark not to cost enough to justify an exception to that rule (see
[ADR-0012](https://github.com/tokuchan/bridge/blob/master/docs/adr/0012-format-print-truss-owns-the-facility.md)).

Scope is deliberately narrower than the full standard surface: no range
or chrono formatting, no locale-aware formatting (the `L` flag parses
but is ignored), and no compile-time format-string validation (C++17
has no `consteval` to do that with -- a malformed literal format string
compiles and throws at runtime here, instead of failing to compile).
Each gap is disclosed in ADR-0012, and the runtime-vs-compile-time
validation gap specifically carries a
[`BRIDGE_RIVETS_DIVERGENCE_NOTE`](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md)
wherever the polyfill's `format_string` is actually selected.

### `formatter<T>` needs two specializations to survive a toolchain upgrade

Every other symbol on this page (`format_error`, `format_context`,
`format_parse_context`, ...) is a plain alias Deck resolves per path,
transparently either way. `formatter<T>` can't work the same way: it's
the one symbol here users *specialize*, and C++ doesn't allow
specializing an alias template. Extending formatting for your own type
means specializing `bridge::truss::formatter<T>` (for the polyfill
path) or `std::formatter<T>` (once passthrough activates) directly --
not `bridge::formatter<T>` itself, which can be named but never
specialized. A type meant to stay formattable across a C++17-&gt;C++20
toolchain upgrade genuinely needs both specializations; this is an
inherent limitation of what alias templates can do in C++, disclosed
in ADR-0012's own subsection on the topic, not a gap in this facility's
design.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `format` | function | Formats args according to fmt, returning the result as a new std::string. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_args` | class | Type-erased argument pack for vformat, constructed via make_format_args. Narrower in scope than real std::format_args: pinned to format_context&lt; std::back_insert_iterator&lt;std::string&gt;&gt; rather than generic over any output iterator, since this polyfill's scope doesn't include vformat_to (only plain vformat, which always targets std::string) a direct, disclosed consequence of docs/adr/0012's scope decision, not a separate divergence in its own right. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_context` | class | The output context passed to formatter&lt;T&gt;::format: wherever formatted output goes (OutIt), plus access to dynamic width/precision resolution. Matches real std::basic_format_context&lt;OutIt, char&gt;'s essential shape — a user's formatter&lt;T&gt;::format written against this type is source-compatible with the real one. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_error` | class | Thrown when a format string is malformed, references an out-of-range or type-mismatched argument, or otherwise can't be honored. Matches std::format_error. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_parse_context` | class | Cursor over the not-yet-consumed portion of a replacement field's format-spec substring, plus the auto/manual argument-indexing state shared across the whole format string. Matches real std::format_parse_context's begin()/end()/advance_to()/next_arg_id()/ check_arg_id() shape — a user's formatter&lt;T&gt;::parse written against this type is source-compatible with the real one. Not constexpr here (unlike the real type): this polyfill never evaluates format strings at compile time in the first place (no consteval pre-C++20 to do so with — see docs/adr/0012's disclosed compile-time-validation gap), so there's nothing to gain from it. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_string` | class | A format string, implicitly constructible from anything convertible to std::string_view (matching real std::format_string&lt;Args...&gt;'s converting-constructor shape). Unlike the real type, this constructor does not validate the format string at compile time C++17 has no consteval to do that with (docs/adr/0012's disclosed compile-time-validation gap). Validation happens only when the string is actually used to format, at which point a malformed spec throws format_error same as vformat's contract always has. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to` | function | Formats args according to fmt, writing to out. Generic over any output iterator, matching the real signature (not a curated fixed set of sinks). | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to_n` | function | Formats args according to fmt, writing at most n characters to out. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `format_to_n_result` | struct | The result of format_to_n: out is an iterator one past the last element actually written; size is the total number of characters that would have been written for an unlimited output size, matching real std::format_to_n_result. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `formatted_size` | function | Computes the length format(fmt, args...) would produce, without building the string. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `formatter` | struct | The formatter&lt;T&gt; customization point, disabled by default — matching real std::formatter's "disabled formatter" behavior for any T without a specialization. Attempting to use an unformattable type fails to compile with a reasonably clear "deleted function" error rather than a wall of SFINAE errors, via the deleted default constructor. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `make_format_args` | function | Constructs a type-erased format_args from args, for vformat. Matches real std::make_format_args' shape (lvalue references called with the named parameters of whatever function is forwarding into vformat, which are themselves lvalues regardless of the original argument's value category). The result is only valid for the duration of the full expression that calls vformat with it, same lifetime constraint as the real function. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `print` | function | Writes format(fmt, args...) to stream. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `println` | function | Writes format(fmt, args...) followed by a newline to stream. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
| `vformat` | function | Formats a type-erased argument pack according to fmt, returning the result as a new std::string. Unlike format, fmt is a plain std::string_view vformat is explicitly the "runtime format string, no format-string- specific type" entry point, matching real std::vformat. | [https://en.cppreference.com/w/cpp/utility/format](https://en.cppreference.com/w/cpp/utility/format) |
<!-- BRIDGE-DOCS:END -->
