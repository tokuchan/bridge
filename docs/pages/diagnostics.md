\page page_diagnostics Diagnostics

<!-- BRIDGE-DOCS:BEGIN header-link -->
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/rivets/diagnostics.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`BRIDGE_RIVETS_DIVERGENCE_NOTE("...")` emits a compiler-visible
`#pragma message` at the point of expansion -- the mechanism every
other facility page's own Divergences section relies on to surface a
disclosed, surprising gap *at the point it matters*, not just in an
ADR a consumer has to go spelunking for.

## Example

```cpp
#include <rivets/diagnostics.hpp>

BRIDGE_RIVETS_DIVERGENCE_NOTE("bridge::widget (polyfill): rounds down instead of to nearest, see docs/adr/0099.")

int main() {}
```

Placed where the *diverging* code path is actually selected (a Deck
header's polyfill branch), so it only fires for a translation unit
actually on that path.

## Notes

- Verified to compile cleanly under `-Wall -Wextra -Werror` on both GCC
  (a note, immune to `-Werror` by construction) and Clang (a warning
  under `-W#pragma-messages`, but doesn't trigger `-Werror`).
- Not every gap gets one: a limitation that's simply absent
  functionality gets a normal, self-explanatory compile error instead.
  See [ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md)
  for the full policy on which kinds of gaps qualify.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_RIVETS_DIVERGENCE_NOTE` | define | Emits `#pragma message` with `msg` at the point of expansion. |
| `BRIDGE_RIVETS_PRAGMA` | define | Turns a token sequence into a `_Pragma` string-literal argument. Exposition-only: use BRIDGE_RIVETS_DIVERGENCE_NOTE directly instead. |
<!-- BRIDGE-DOCS:END -->
