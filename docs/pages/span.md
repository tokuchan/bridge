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
sequence of `T`, matching C++20's `std::span<T, Extent>`. `std::span`
doesn't exist at all before C++20 -- like `expected`, there's no
pre-existing C++17 type for Truss to attach free functions onto, so
Truss owns a from-scratch class, with both static and dynamic `Extent`
supported in full (including the storage-layout difference: a
fixed-`Extent` span stores no runtime size at all).

## Example

```cpp
#include <deck/cpp17/span.hpp>

std::vector<int> data = {1, 2, 3, 4, 5};
bridge::span<int> s(data);

s.first(2)[1]; // == 2
s.last(2)[0];  // == 4
bridge::as_bytes(s).size(); // == 5 * sizeof(int)
```

## Divergences

- **Range/container-constructing overloads cover only the specific
  named cases real code overwhelmingly needs** -- a pointer+count, an
  iterator pair, a C array, `std::array`, `std::vector`, `std::string`
  -- rather than a generic SFINAE net approximating C++20's
  `ranges::contiguous_range`. A custom container type outside this list
  fails to compile under the polyfill; **not treated as a disclosed
  divergence** ([ADR-0015](https://github.com/tokuchan/bridge/blob/master/docs/adr/0015-span-truss-owns-the-class.md)):
  it fails the same loud, compile-time way real `std::span` would for a
  genuinely non-contiguous type, not a silent runtime surprise.
- **The C++23 tuple-like interface** (`get<I>(span)`, structured
  bindings via `std::tuple_size`/`std::tuple_element`) is out of scope,
  deferred as its own disclosed follow-up with an independent Feature
  Test threshold once a compiler in this project's matrix actually
  reports it.

## Passthrough

Deck aliases to real `std::span<T, Extent>` once
`BRIDGE_RIVETS_FEATURES_LIB_SPAN` confirms native support, or to
`bridge::truss::span<T, Extent>` otherwise -- Truss's own class never
itself passes through, even under C++20/23.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `as_bytes` | function | Reinterprets `s`'s elements as a read-only view of their underlying bytes. Matches real `std::as_bytes`. | [`<span>::as_bytes`](https://en.cppreference.com/w/cpp/container/span) |
| `as_writable_bytes` | function | Reinterprets `s`'s elements as a writable view of their underlying bytes. Matches real `std::as_writable_bytes`. Only participates when `T` isn't itself const-qualified, matching the real function's constraint. | [`<span>::as_writable_bytes`](https://en.cppreference.com/w/cpp/container/span) |
| `dynamic_extent` | variable | Polyfill companion to span. | [`<span>::dynamic_extent`](https://en.cppreference.com/w/cpp/container/span) |
| `span` | class | A non-owning view over a contiguous sequence of `T`, with an `Extent` known either at compile time or only at runtime. Matches real `std::span<T, Extent>`'s shape: trivially copyable, never allocates, never extends the viewed sequence's lifetime. | [`<span>::span`](https://en.cppreference.com/w/cpp/container/span) |
<!-- BRIDGE-DOCS:END -->
