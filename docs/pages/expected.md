\page page_expected Expected

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<expected>`](https://en.cppreference.com/w/cpp/utility/expected) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/expected.hpp`
- `include/deck/cpp17/expected.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::expected<T,E>` holds either a value of type `T` or an error of
type `E`, matching C++23's `std::expected<T,E>`. `std::expected` doesn't
exist before C++23 and has no pre-existing STL type to attach free
functions to (unlike `optional`), so Truss owns a complete,
from-scratch class -- including its companions `unexpected<E>`,
`unexpect_t`/`unexpect`, and `bad_expected_access<E>` -- and Deck
selects between it and the real `std::expected<T,E>`.

## Example

```cpp
#include <deck/cpp17/expected.hpp>

bridge::expected<int, std::string> half(int v) {
    if (v % 2 != 0) {
        return bridge::expected<int, std::string>{bridge::unexpect, "odd"};
    }
    return bridge::expected<int, std::string>{v / 2};
}

bridge::expected<int, std::string> start{8};
auto result = start.and_then(half).and_then(half);
// result == bridge::expected<int, std::string>{2}

bridge::expected<int, std::string> odd{7};
auto failed = odd.and_then(half);
// failed.error() == "odd"
```

Both return statements construct `expected` explicitly rather than
relying on an implicit conversion from `unexpected<E>` or a raw value --
see Divergences below for why that matters here specifically.

## Divergences

- **Converting constructors are unconditionally `explicit`.** Real
  `std::expected` allows implicit conversion from `unexpected<G>`, a raw
  value, or another `expected<U,G>` when convertibility permits; C++17
  has no `explicit(bool)` to make that conditional, so the polyfill
  requires explicit construction everywhere instead.
- **The `expected<U,G>` converting constructor omits `std::expected`'s
  extra defensive SFINAE guards** (converts-from-any-cvref and
  friends), matching only the core `is_constructible_v` constraint --
  an edge-case ambiguity real `std::expected` guards against that the
  polyfill doesn't.
- **No conditional triviality.** `std::expected<T,E>` is trivially
  copyable/destructible when `T`/`E` are; replicating that needs the
  same base-class-specialization machinery real STL implementations
  use, out of scope for a header-only backport.
- **Simplified exception safety.** Assignment uses straightforward
  destroy-then-construct, not the standard's full two-stage
  "reinit-expected" technique for the narrow case where a throwing move
  constructor could otherwise leave the object valueless after a failed
  reassignment.

The first two are disclosed via a compiler-visible
`BRIDGE_RIVETS_DIVERGENCE_NOTE` wherever the polyfill is actually
selected ([ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md));
all four are covered in full in
[ADR-0010](https://github.com/tokuchan/bridge/blob/master/docs/adr/0010-expected-truss-owns-the-class.md).

## Passthrough

Deck selects between passthrough and polyfill on
`BRIDGE_RIVETS_FEATURES_LIB_EXPECTED >= 202211L` (the Feature Test
value for `std::expected`, [P0323](https://en.cppreference.com/w/cpp/feature_test)):
`std::expected<T,E>` once confirmed, `bridge::truss::expected<T,E>`
otherwise -- Truss's own class never itself passes through, even under
C++23. There's nothing left for Deck to *wrap* here, unlike `optional`:
Deck only builds a wrapper when Truss's contribution is free functions
on an existing STL type, not a full class.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `bad_expected_access` | class | Thrown by `expected<T,E>::value()` when accessed without a value; carries a copy of the error that caused the access to fail. Matches `std::bad_expected_access<E>`. | [`<expected>::bad_expected_access`](https://en.cppreference.com/w/cpp/utility/expected) |
| `expected` | class | Truss's polyfilled `expected<T,E>`, matching C++23's `std::expected<T,E>` for standards that predate it. | [`<expected>::expected`](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect` | variable | The canonical `unexpect_t` instance, passed to select `expected`'s in-place-error constructors. Matches `std::unexpect`. | [`<expected>::unexpect`](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect_t` | struct | Tag type selecting `expected`'s error-constructing constructor overloads, mirroring `std::in_place_t`. Matches `std::unexpect_t`. | [`<expected>::unexpect_t`](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpected` | class | Wraps an error value of type `E`, matching `std::unexpected`. Constructed explicitly and passed to `expected`'s converting constructors, or returned directly from a function reporting failure. | [`<expected>::unexpected`](https://en.cppreference.com/w/cpp/utility/expected) |
<!-- BRIDGE-DOCS:END -->
