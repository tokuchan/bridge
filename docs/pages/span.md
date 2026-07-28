\page page_span Span

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<span>`](https://en.cppreference.com/w/cpp/container/span) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/span.hpp`
- `include/deck/cpp17/span.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::span<T, Extent>` is a non-owning view over a contiguous
sequence of `T`. It matches C++20's `std::span<T, Extent>`.

`std::span` does not exist at all before C++20. Like `expected`,
there is no C++17 type for Truss to attach free functions onto, so
Truss owns a class built from scratch. This class supports both
static and dynamic `Extent` in full. This includes the storage-layout
difference: a fixed-`Extent` span stores no runtime size at all.

## Example

```cpp
#include <deck/cpp17/span.hpp>
#include <vector>
#include <cassert>

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5};
    bridge::span<int> s(data);

    assert(s.first(2)[1] == 2);
    assert(s.last(2)[0] == 4);
    assert(bridge::as_bytes(s).size() == 5 * sizeof(int));
}
```

## Divergences

- **The range/container-constructing overloads cover only specific
  named cases.** These cases are the ones real code needs most: a
  pointer plus a count, an iterator pair, a C array, `std::array`,
  `std::vector`, and `std::string`. This facility does not use a
  generic check for any C++20 `ranges::contiguous_range`. A custom
  container type outside this list fails to compile under the
  polyfill.
  [ADR-0015](https://github.com/tokuchan/bridge/blob/master/docs/adr/0015-span-truss-owns-the-class.md)
  does not treat this as a disclosed divergence. Real `std::span`
  fails the same loud, compile-time way for a genuinely
  non-contiguous type. This is not a silent runtime surprise.
- **The C++23 tuple-like interface is out of scope.** This interface
  is `get<I>(span)` plus structured bindings, through
  `std::tuple_size` and `std::tuple_element`. This is a disclosed,
  deferred follow-up. This facility will add its own Feature Test
  threshold once a compiler in this project's matrix actually reports
  one.

## Passthrough

Deck aliases to real `std::span<T, Extent>`, once
`BRIDGE_RIVETS_FEATURES_LIB_SPAN` confirms native support. Otherwise,
Deck aliases to `bridge::truss::span<T, Extent>`. Truss's own class
never itself passes through, even under C++20 or C++23.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `as_bytes` | function | This function reinterprets `s`'s elements as a read-only view of their underlying bytes. This matches real `std::as_bytes`. | [`<span>::as_bytes`](https://en.cppreference.com/w/cpp/container/span) |
| `as_writable_bytes` | function | This function reinterprets `s`'s elements as a writable view of their underlying bytes. This matches real `std::as_writable_bytes`. | [`<span>::as_writable_bytes`](https://en.cppreference.com/w/cpp/container/span) |
| `dynamic_extent` | variable | This is the polyfill companion to span. | [`<span>::dynamic_extent`](https://en.cppreference.com/w/cpp/container/span) |
| `span` | class | This class is a non-owning view over a contiguous sequence of `T`. `Extent` is known either at compile time or only at runtime. | [`<span>::span`](https://en.cppreference.com/w/cpp/container/span) |
<!-- BRIDGE-DOCS:END -->
