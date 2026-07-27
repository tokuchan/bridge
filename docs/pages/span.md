\page page_span Span

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/span.hpp`
<!-- BRIDGE-DOCS:END -->

`std::span<T, Extent>` doesn't exist at all before C++20 -- like
`expected`, there's no pre-existing C++17 type for Truss to attach free
functions onto, so Truss owns a from-scratch class (Deck aliases to
real `std::span` once `BRIDGE_RIVETS_FEATURES_LIB_SPAN` confirms native
support). Both static and dynamic `Extent` are supported in full,
including the storage-layout difference (a fixed-`Extent` span stores
no runtime size at all) and the exact static/dynamic conversion rules
(implicit when going from a known size to a compatible one, explicit
only when narrowing from an unknown-at-compile-time source extent to a
known one). Range/container-constructing overloads are implemented for
the specific named cases real code overwhelmingly needs -- a
pointer+count, an iterator pair, a C array, `std::array`,
`std::vector`, `std::string` -- rather than a generic SFINAE net
approximating C++20's `ranges::contiguous_range`. See
[ADR-0015](https://github.com/tokuchan/bridge/blob/master/docs/adr/0015-span-truss-owns-the-class.md)
for the full fidelity scope, including why a custom container type
outside the named list failing to compile under the polyfill isn't a
disclosed-divergence case (it fails the same loud, compile-time way
real `std::span` would for a genuinely non-contiguous type), and the
C++23 tuple-like interface (`get<I>`, structured bindings) deferred as
its own disclosed follow-up with an independent Feature Test threshold.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `as_bytes` | function | Reinterprets s's elements as a read-only view of their underlying bytes. Matches real std::as_bytes. | [https://en.cppreference.com/w/cpp/container/span](https://en.cppreference.com/w/cpp/container/span) |
| `as_writable_bytes` | function | Reinterprets s's elements as a writable view of their underlying bytes. Matches real std::as_writable_bytes. Only participates when T isn't itself const-qualified, matching the real function's constraint. | [https://en.cppreference.com/w/cpp/container/span](https://en.cppreference.com/w/cpp/container/span) |
| `dynamic_extent` | variable | Sentinel Extent value meaning "the size is only known at runtime." Matches real std::dynamic_extent. | [https://en.cppreference.com/w/cpp/container/span](https://en.cppreference.com/w/cpp/container/span) |
| `span` | class | A non-owning view over a contiguous sequence of T, with an Extent known either at compile time or only at runtime. Matches real std::span&lt;T, Extent&gt;'s shape: trivially copyable, never allocates, never extends the viewed sequence's lifetime. | [https://en.cppreference.com/w/cpp/container/span](https://en.cppreference.com/w/cpp/container/span) |
<!-- BRIDGE-DOCS:END -->
