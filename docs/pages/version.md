\page page_version Version

<!-- BRIDGE-DOCS:BEGIN header-link -->
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/version.hpp`
- `include/deck/cpp17/version.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

Plain library version metadata for Truss and Deck themselves -- not a
detection or polyfill facility, and not tied to any `std::` equivalent.
Doesn't fit naturally under any of the other facilities, so it gets its
own small one here rather than being shoehorned into `optional` or
`expected` just because it happens to be a small header.

## Example

```cpp
#include <deck/cpp17/version.hpp>
#include <cassert>

int main() {
    bridge::deck::version_info v{};
    assert(v.major == BRIDGE_DECK_VERSION_MAJOR);
    assert(v.truss.major == BRIDGE_TRUSS_VERSION_MAJOR);
}
```

## Notes

See [ADR-0005](https://github.com/tokuchan/bridge/blob/master/docs/adr/0005-calver-versioning.md)
for how these numbers relate to the project's actual CalVer release
versioning -- they're independent: this is Truss's/Deck's own internal
version metadata, not the release version a package manager would see.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_DECK_VERSION_MAJOR` | define | Deck's major version number. |
| `BRIDGE_DECK_VERSION_MINOR` | define | Deck's minor version number. |
| `BRIDGE_DECK_VERSION_PATCH` | define | Deck's patch version number. |
| `BRIDGE_TRUSS_VERSION_MAJOR` | define | Truss's major version number. |
| `BRIDGE_TRUSS_VERSION_MINOR` | define | Truss's minor version number. |
| `BRIDGE_TRUSS_VERSION_PATCH` | define | Truss's patch version number. |
| `version_info` | struct | Deck's version, plus the Truss version it was built against. |
<!-- BRIDGE-DOCS:END -->
