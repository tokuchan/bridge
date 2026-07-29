\page page_source_location Source Location

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<source_location>`](https://en.cppreference.com/w/cpp/utility/source_location) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/source_location.hpp`
- `include/deck/cpp17/source_location.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::source_location` holds one point in source code: a file
name, a function name, a line number, and a column number. It matches
C++20's `std::source_location`.

`std::source_location` does not exist at all before C++20. Like
`expected`, `span`, and `jthread`, there is no C++17 type for Truss to
attach free functions onto, so Truss owns a class built from scratch.
`source_location::current()` captures its caller's call site. Call
`current()` as a default argument. The default argument evaluates at
the call site, not inside the function it defaults for.

## Example

```cpp
#include <deck/cpp17/source_location.hpp>
#include <cassert>
#include <cstring>

void log_call(bridge::source_location loc = bridge::source_location::current()) {
    assert(std::strlen(loc.file_name()) > 0);
    assert(loc.line() > 0);
}

int main() { log_call(); }
```

## Divergences

- **`column()` reports `0` on some toolchains.** GCC has never
  implemented a public compiler builtin for a call site's column
  number, on any version. This polyfill's `column()` falls back to a
  fixed `0` there. Clang has had this builtin since Clang 9, well
  below this project's matrix floor, so this polyfill's `column()`
  reports the real column on Clang.
  [ADR-0019](https://github.com/tokuchan/bridge/blob/master/docs/adr/0019-source-location-truss-owns-the-class.md)
  has the full rationale, including why this gap is unavoidable: real
  `std::source_location` gets a correct column on GCC only through
  libstdc++'s own private, non-portable builtin, one a third-party
  polyfill cannot use.
- **`function_name()` reports a bare name, not a full signature.**
  This polyfill uses the only portable builtin available,
  `__builtin_FUNCTION()`, which returns a bare name, for example
  `"main"`. Real `std::source_location::function_name()` returns a
  full function signature on libstdc++, for example `"int main()"` —
  this project's whole compiler matrix uses libstdc++, so this gap
  applies everywhere in this project's own matrix, not just on one
  compiler. See ADR-0019 for why a closer match was rejected.

Both gaps apply only while the polyfill is active. Real
`std::source_location`, once Deck selects it, reports the true value
for both.

## Passthrough

Deck aliases to real `std::source_location`, once
`BRIDGE_RIVETS_FEATURES_LIB_SOURCE_LOCATION` confirms native support.
Otherwise, Deck aliases to `bridge::truss::source_location`. This
Feature Test needs no Detector-backed override — a direct compiler
probe confirmed it reports accurately on every toolchain this
project's matrix covers. Truss's own class never itself passes
through, even under C++20.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH` | define | This macro tells you whether `source_location` should pass through to the real `std::source_location`. This macro is a bare Feature Test check. | [`<source_location>::BRIDGE_DECK_SOURCE_LOCATION_PASSTHROUGH`](https://en.cppreference.com/w/cpp/utility/source_location) |
| `source_location` | class | This class holds one point in source code: a file name, a function name, a line number, and a column number. This class matches `std::source_location`. | [`<source_location>::source_location`](https://en.cppreference.com/w/cpp/utility/source_location) |
<!-- BRIDGE-DOCS:END -->
