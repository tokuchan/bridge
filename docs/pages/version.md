\page page_version Version

<!-- BRIDGE-DOCS:BEGIN header-link -->
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/version.hpp`
- `include/deck/cpp17/version.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

This facility holds plain version metadata, for Truss and Deck
themselves. This facility is not a detection facility. This facility
is not a polyfill facility. This facility has no `std::` equivalent.

This metadata does not fit naturally under any other facility. It
gets its own small page here instead.

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
for how these numbers relate to the project's own CalVer release
version. The two are independent. This page's version numbers are
Truss's and Deck's own internal metadata. A package manager sees a
different number: the project's release version.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief |
|---|---|---|
| `BRIDGE_DECK_VERSION_MAJOR` | define | This is Deck's major version number. |
| `BRIDGE_DECK_VERSION_MINOR` | define | This is Deck's minor version number. |
| `BRIDGE_DECK_VERSION_PATCH` | define | This is Deck's patch version number. |
| `BRIDGE_TRUSS_VERSION_MAJOR` | define | This is Truss's major version number. |
| `BRIDGE_TRUSS_VERSION_MINOR` | define | This is Truss's minor version number. |
| `BRIDGE_TRUSS_VERSION_PATCH` | define | This is Truss's patch version number. |
| `version_info` | struct | This struct holds Deck's version. It also holds the Truss version Deck was compiled against. |
<!-- BRIDGE-DOCS:END -->
