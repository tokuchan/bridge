\page page_expected Expected

<!-- BRIDGE-DOCS:BEGIN header-link -->
See [`<expected>`](https://en.cppreference.com/w/cpp/utility/expected) on cppreference.
<!-- BRIDGE-DOCS:END -->

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/expected.hpp`
- `include/deck/cpp17/expected.hpp`
<!-- BRIDGE-DOCS:END -->

## Synopsis

`bridge::expected<T,E>` holds either a value of type `T`, or an error
of type `E`. It matches C++23's `std::expected<T,E>`.

`std::expected` does not exist before C++23. It also has no STL type
to attach free functions to, unlike `optional`. So Truss owns a
complete class, built from scratch. This class includes its
companions: `unexpected<E>`, `unexpect_t`, `unexpect`, and
`bad_expected_access<E>`. Deck selects between this class and the
real `std::expected<T,E>`.

## Example

```cpp
#include <deck/cpp17/expected.hpp>
#include <string>
#include <cassert>

bridge::expected<int, std::string> half(int v) {
    if (v % 2 != 0) {
        return bridge::expected<int, std::string>{bridge::unexpect, "odd"};
    }
    return bridge::expected<int, std::string>{v / 2};
}

int main() {
    bridge::expected<int, std::string> start{8};
    auto result = start.and_then(half).and_then(half);
    assert(result.has_value());
    assert(*result == 2);

    bridge::expected<int, std::string> odd{7};
    auto failed = odd.and_then(half);
    assert(!failed.has_value());
    assert(failed.error() == "odd");
}
```

Both return statements construct `expected` explicitly. They do not
rely on an implicit conversion from `unexpected<E>` or a raw value.
See Divergences below for why that matters here.

## Divergences

- **Converting constructors are unconditionally `explicit`.** Real
  `std::expected` allows an implicit conversion, from `unexpected<G>`,
  from a raw value, or from another `expected<U,G>`, when
  convertibility permits. C++17 has no `explicit(bool)` to make that
  conditional. So the polyfill requires explicit construction
  everywhere instead.
- **The `expected<U,G>` converting constructor omits
  `std::expected`'s extra defensive SFINAE guards.** These guards
  include the converts-from-any-cvref check and similar checks. This
  constructor matches only the core `is_constructible_v` constraint.
  Real `std::expected` guards against an edge-case ambiguity here.
  The polyfill does not guard against it.
- **No conditional triviality.** `std::expected<T,E>` is trivially
  copyable and trivially destructible, when `T` and `E` are.
  Reproducing this needs the same base-class-specialization machinery
  real STL implementations use. This machinery is out of scope for a
  header-only backport.
- **Simplified exception safety.** Assignment uses straightforward
  destroy-then-construct. The standard uses a full two-stage
  technique instead, called "reinit-expected." The standard's
  technique covers one narrow case: a throwing move constructor could
  otherwise leave the object valueless, after a failed reassignment.

This page discloses the first two divergences with a
compiler-visible `BRIDGE_RIVETS_DIVERGENCE_NOTE`, wherever the
polyfill is actually selected
([ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md)).
[ADR-0010](https://github.com/tokuchan/bridge/blob/master/docs/adr/0010-expected-truss-owns-the-class.md)
covers all four divergences in full.

## Passthrough

Deck checks one Feature Test:
`BRIDGE_RIVETS_FEATURES_LIB_EXPECTED >= 202211L`. This is the Feature
Test value for `std::expected`
([P0323](https://en.cppreference.com/w/cpp/feature_test)). Once this
check confirms support, Deck aliases to `std::expected<T,E>`.
Otherwise, Deck aliases to `bridge::truss::expected<T,E>`. Truss's own
class never itself passes through, even under C++23.

Deck has nothing left to wrap here, unlike `optional`. Deck only
builds a wrapper when Truss's contribution is free functions on an
existing STL type, not a full class.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `bad_expected_access` | class | `expected<T,E>::value()` throws this class, when you access it without a value. This class carries a copy of the error that caused the access to fail. This class matches `std::bad_expected_access<E>`. | [`<expected>::bad_expected_access`](https://en.cppreference.com/w/cpp/utility/expected) |
| `expected` | class | This class is Truss's polyfilled `expected<T,E>`. This class matches C++23's `std::expected<T,E>`, for standards that predate it. | [`<expected>::expected`](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect` | variable | This is the canonical `unexpect_t` instance. You pass this instance to select `expected`'s in-place-error constructors. This instance matches `std::unexpect`. | [`<expected>::unexpect`](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect_t` | struct | This tag type selects `expected`'s error-constructing constructor overloads. This tag type mirrors `std::in_place_t`. This tag type matches `std::unexpect_t`. | [`<expected>::unexpect_t`](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpected` | class | This class wraps an error value of type `E`. This class matches `std::unexpected`. | [`<expected>::unexpected`](https://en.cppreference.com/w/cpp/utility/expected) |
<!-- BRIDGE-DOCS:END -->
