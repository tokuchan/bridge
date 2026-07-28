\page page_diagnostics Diagnostics

<!-- BRIDGE-DOCS:BEGIN header-link -->
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/rivets/diagnostics.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`BRIDGE_RIVETS_DIVERGENCE_NOTE("...")` emits a compiler-visible
`#pragma message`, at the point where it expands. Every other facility
page's Divergences section relies on this macro. This macro shows a
disclosed, surprising gap at the point it matters. A consumer does not
need to search an ADR to find it.

## Example

```cpp
#include <rivets/diagnostics.hpp>

BRIDGE_RIVETS_DIVERGENCE_NOTE("bridge::widget (polyfill): rounds down instead of to nearest, see docs/adr/0099.")

int main() {}
```

Place this macro where the diverging code path is actually selected,
for example inside a Deck header's polyfill branch. This macro then
only fires for a translation unit on that path.

## Notes

- This macro compiles cleanly under `-Wall -Wextra -Werror`, on both
  GCC and Clang. On GCC, `#pragma message` is a note. A note is immune
  to `-Werror`. On Clang, `#pragma message` is a warning under
  `-W#pragma-messages`. This warning does not trigger `-Werror`.
- Not every gap gets a note. A limitation that is simply missing
  functionality gets a normal compile error instead. See
  [ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md)
  for the full policy on which gaps qualify.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_RIVETS_DIVERGENCE_NOTE` | define | This macro emits `#pragma message` with `msg`, at the point where this macro expands. |
| `BRIDGE_RIVETS_PRAGMA` | define | This macro turns a token sequence into a `_Pragma` string-literal argument. |
<!-- BRIDGE-DOCS:END -->
