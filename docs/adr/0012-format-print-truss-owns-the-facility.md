# For `format`/`print`, Truss Owns the Whole Facility, Gated Independently

## Context

`std::format` breaks the precondition both prior Truss/Deck shapes
depend on. It isn't an existing STL type Truss can extend with free
functions (`optional`'s shape), and it isn't a single type Truss can
own as a from-scratch class (`expected`'s shape, [ADR-0010](0010-expected-truss-owns-the-class.md)):
it's function-shaped — `format`, `format_to`, `format_to_n`,
`formatted_size`, `vformat` — built on a parser, a type-erasure layer,
and a user-extensible `formatter<T>` customization point, with no
pre-existing STL facility at all before C++20. `std::print`/
`std::println` compound this: they don't exist until C++23, three
years after `format` itself, and depend on `format`'s machinery
internally.

## Decision

Truss owns the whole facility — the format-spec parser, the contexts,
`formatter<T>` and its built-in specializations, the top-level entry
points, and `print`/`println` built on top. Same "Truss never passes
through, only Deck selects" invariant as `expected`, applied to a
function-based facility instead of a class.

### Independent Feature Tests

`format` and `print` cross their real passthrough thresholds at
different standards — confirmed by direct compiler probe, not
assumed: `std::format` is usable from `-std=c++20` on GCC 15.3/Clang
20 (`__cpp_lib_format` defined, value `202304L` on this pair); the
`<print>` *header* is includable even under `-std=c++20`, but
`std::print`/`std::println` themselves are hard-rejected until
`-std=c++23` ("only available from C++23 onwards"), matching
`__cpp_lib_print`'s absence before that mode. Deck therefore gates
`format`'s passthrough on `BRIDGE_RIVETS_FEATURES_LIB_FORMAT` and
`print`'s on `BRIDGE_RIVETS_FEATURES_LIB_PRINT` independently, rather
than sharing one gate — an ecosystem can have native `format` without
native `print`, and Deck's selection needs to reflect that per-feature,
not lump them together.

### `print` is always built on Truss's own `format`

`bridge::truss::print`/`println` call into this same library's
`format.hpp`, never into whichever `format` Deck happened to select —
uniform with `expected`'s "Truss never passes through" rule, applied
across two features that Truss itself composes internally, not just
within one.

This was explicitly raised as an open question during design (a
C++20-but-pre-C++23 ecosystem has native `format` `print` could
theoretically borrow for efficiency) and left provisional pending a
benchmark before being treated as locked in. The benchmark: a
representative print-shaped workload (a mixed-type format string —
int, float with precision, string, hex) run 200,000 times through both
Truss's own `format` and native `std::format`, under `-O2`, on both
GCC and Clang, each compiler's numbers checked across three runs for
consistency. Result: Truss's polyfill is consistently **~15–30%
slower** per call (roughly 30–40ns of actual difference, both paths
well under 200ns total) — a real, measurable, but modest gap, not the
order-of-magnitude difference that would justify a carved-out
exception to an architectural invariant applied consistently
everywhere else in this codebase. **Confirmed final**: `print` always
uses Truss's own `format`, unconditionally.

### Scope

Full C++23 `format`/`print` is one of the largest single features in
the standard library — range formatting, chrono formatting,
locale-aware formatting, and compile-time format-string checking are
each substantial on their own. This pass covers:

- `format`, `format_to` (generic over any output iterator, matching
  the real signature), `format_to_n`, `formatted_size`, `vformat`,
  `format_error`
- `print`/`println`, both `FILE*`-targeting and `ostream`-targeting
  overload families
- The full two-phase `formatter<T>` customization point
  (`parse(ParseContext&)` + `format(const T&, FormatContext&) const`,
  templated on the context type) for the standard formattable
  built-ins: integral types, `bool`, floating point, `char`,
  C-string/`std::string`/`string_view`, pointers/`nullptr_t`
- The full format-spec mini-language minus locale: fill-and-align,
  sign, `#`, `0`, width and precision (literal or dynamic via
  `{}`/`{N}`), and the per-type-family presentation-type characters

**Explicitly out of scope**, each a disclosed, follow-up-worthy gap:

- Range formatting (C++23 — formatting `vector`/`map`/etc. directly)
- Chrono formatting
- Locale-aware formatting — the `L` spec flag is accepted
  syntactically (a format string using it still parses) but always
  ignored, behaving as if absent, never rejected
- `wchar_t`/wide-character formatting entirely — narrow char/UTF-8
  only, matching this project's existing scope (neither `optional` nor
  `expected` touch wide chars either)
- Field-width computed by Unicode display-column estimation. Real
  `std::format` measures a field's width against Unicode text
  segmentation rules (roughly: one column per grapheme cluster, not
  per byte). This polyfill measures width in bytes — correct for
  ASCII content, an approximation for multi-byte UTF-8 content, where
  a computed field would come out wider than intended
- Compile-time format-string validation — real `std::format` uses a
  `consteval` constructor on `basic_format_string` to reject
  provably-invalid literal format strings at compile time; C++17 has
  no `consteval`, so this genuinely cannot be replicated. The polyfill
  validates only at runtime (throwing `format_error`), matching
  `vformat`'s contract but not literal-format-string compile-time
  rejection — disclosed per [ADR-0011](0011-warn-on-surprising-facility-divergences.md)
  via a `BRIDGE_RIVETS_DIVERGENCE_NOTE` where the polyfill's
  `format_string`-equivalent type is selected

## Consequences

- `CONTEXT.md`'s Truss entry gains a third shape: a complete
  function-based facility (not just a class) when there's nothing to
  extend at all, plus the rule that a facility Truss builds on its own
  other facilities (`print` on `format`) never passes through
  independently — only Deck's ultimate selection for each individually
  gated feature does.
- Deck needs two independent alias-selection headers
  (`deck/cpp17/format.hpp`, `deck/cpp17/print.hpp`), each with its own
  Feature Test gate, rather than one combined selection — the first
  time two features in the same ADR cross their thresholds separately.
- A differential test comparing Truss's polyfill against real
  `std::format`/`std::print` directly is possible in one C++20/23
  translation unit, same technique as `expected`'s: since Truss never
  passes through, the polyfill and the real facility coexist as
  distinct things to compare against each other rather than testing
  each path in isolation and hoping they agree.
- The byte-based field-width approximation means a consumer relying on
  precise column alignment for multi-byte UTF-8 content will observe a
  difference between the polyfill and passthrough paths — accepted as
  a known, documented limitation rather than pulling in Unicode
  text-segmentation logic for a header-only backport.
