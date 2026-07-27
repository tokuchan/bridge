\page page_diagnostics Diagnostics

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/rivets/diagnostics.hpp`
<!-- BRIDGE-DOCS:END -->

Every Truss-owned polyfill carries some number of deliberate, disclosed
divergences from the real standard facility it backports -- a genuine
language limitation (no `consteval` before C++20, so `format`'s
compile-time format-string validation can't be replicated), or a scope
trim accepted for cost reasons. `BRIDGE_RIVETS_DIVERGENCE_NOTE("...")`
exists because a prose-only record of these (an ADR section, a code
comment) gives a consumer no signal *at the point it matters* -- they
just hit confusing behavior and have to go spelunking to find out why.
It expands to `#pragma message "..."`, placed where the *diverging*
code path is actually selected (a Deck header's polyfill branch, not
unconditionally inside Truss's own header), so it only fires for a
translation unit actually on that path. Verified to compile cleanly
under `-Wall -Wextra -Werror` on both GCC (a note, immune to `-Werror`
by construction) and Clang (a warning under `-W#pragma-messages`, but
didn't trigger `-Werror` in testing). See
[ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md)
for the full policy, including which kinds of gaps *don't* need one
(a limitation that's simply absent functionality gets a normal,
self-explanatory compile error instead).

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_RIVETS_DIVERGENCE_NOTE` | define | Emits `#pragma message` with `msg` at the point of expansion. |
| `BRIDGE_RIVETS_PRAGMA` | define | Turns a token sequence into a `_Pragma` string-literal argument. Exposition-only: use BRIDGE_RIVETS_DIVERGENCE_NOTE directly instead. |
<!-- BRIDGE-DOCS:END -->
