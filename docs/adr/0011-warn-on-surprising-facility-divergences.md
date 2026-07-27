# Warn, Don't Just Document, When a Polyfill's Divergence Could Surprise a User

## Context

Every Truss-owned polyfill so far carries some number of deliberate,
disclosed divergences from the real standard facility it backports —
driven either by a genuine language limitation (e.g. no `consteval`
before C++20, so compile-time validation some real facilities perform
can't be replicated) or a scope trim accepted for cost reasons (e.g.
`expected`'s omitted "reinit-expected" exception-safety technique,
[ADR-0010](0010-expected-truss-owns-the-class.md)). Until now, the
only record of these was prose: an ADR section, a code comment, a
commit message. A consumer who hits one of these edge cases in
practice — code that behaves differently than it would against the
real facility — has no signal *at the point it matters* that they've
crossed a known, documented boundary; they just see confusing,
unexplained behavior and have to go spelunking through ADRs to find
out why.

This was caught mid-design for `std::format`'s runtime-vs-compile-time
format-string validation gap ([ADR-0012](0012-format-print-truss-owns-the-facility.md)):
real `std::format` rejects a provably-invalid literal format string at
compile time via a `consteval` constructor; the polyfill can only
throw `format_error` at runtime. Code that would fail to compile
against the real facility instead compiles fine and throws only when
run — a strictly worse failure mode than the compile error a newer
toolchain would give, and invisible until then.

## Decision

Any Truss-owned facility with a disclosed divergence from the real
standard facility that could reasonably surprise a consumer gets a
compiler-visible note, not just documentation. The mechanism is
`BRIDGE_RIVETS_DIVERGENCE_NOTE("message")`
(`include/rivets/diagnostics.hpp`), a small `_Pragma`-based macro
expanding to `#pragma message "message"`. It's placed where the
*diverging* code path is actually selected for a given translation
unit — inside a Deck header's polyfill branch, not unconditionally
inside Truss's own header, since Truss's header is typically included
regardless of which path Deck ultimately selects and the note is only
relevant to a consumer who's actually on the polyfill path.

Verified empirically before adopting this, not assumed: the macro
compiles cleanly under `-Wall -Wextra -Werror` on both GCC (`#pragma
message` is a note there, immune to `-Werror` by construction) and
Clang (categorized as a warning under `-W#pragma-messages`, but did
not trigger `-Werror` in practice as tested). The macro requires its
message be a single string-literal argument — an unquoted,
comma-containing message splits across multiple macro arguments and
fails to compile, confirmed by hitting exactly that preprocessor error
before settling on the final shape.

This is deliberately narrower than "warn about every documented
limitation": a limitation that's simply *absent* functionality (e.g.
range/chrono/locale formatting not being implemented at all) doesn't
need a note — the consumer gets a normal "no such function" compile
error, which is already self-explanatory. The bar is specifically
divergent *behavior* where code compiles and appears to work but
means something subtly different than it would against the real
facility.

### Retroactive application

Applied to `expected`'s two already-shipped silent divergences, not
left as a gap between this policy and prior work:

- The converting constructors (from `unexpected<G>`, a raw value `U`,
  or `expected<U,G>`) are unconditionally `explicit` in the polyfill,
  where real `std::expected` is conditionally implicit based on
  convertibility (C++17 has no `explicit(bool)` to make this
  conditional).
- The `expected<U,G>` converting constructor omits the standard's
  extra defensive SFINAE guards (`converts-from-any-cvref` and
  friends), matching only the core `is_constructible_v<T, UF>`/
  `is_constructible_v<E, GF>` constraint.

Both notes land in `deck/cpp17/expected.hpp`'s polyfill-selection
branch.

## Consequences

- Every future Truss/Deck facility with a disclosed divergence needs a
  `BRIDGE_RIVETS_DIVERGENCE_NOTE` alongside its ADR entry — this joins
  the existing "read every ADR before committing" check
  (`CLAUDE.md`) as something to verify, not just something to
  remember to write.
- A consumer building with `-Werror` and an unusually strict Clang
  warning configuration (explicitly escalating `-W#pragma-messages` to
  an error) could have their build broken by a note that would
  otherwise be informational only. Accepted as an unlikely, disclosed
  edge case rather than avoided by not warning at all.
- Feature-specific ADRs (starting with `0012`) reference this ADR for
  *how* to disclose a divergence, rather than re-deriving the
  mechanism each time.
