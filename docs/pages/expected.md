\page page_expected Expected

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/expected.hpp`
- `include/deck/cpp17/expected.hpp`
<!-- BRIDGE-DOCS:END -->

`std::expected<T,E>` doesn't exist before C++23, and unlike `optional`
there's no pre-C++23 STL type to attach free functions to -- so Truss
owns a complete, from-scratch class here, matching real `std::expected`
member-for-member (including its companions `unexpected<E>`,
`unexpect_t`/`unexpect`, and `bad_expected_access<E>`). Deck's own
`expected.hpp` then collapses to a plain type alias: either straight to
`std::expected<T,E>` once the ecosystem has it, or to
`bridge::truss::expected<T,E>` otherwise -- there's nothing left for
Deck to *wrap*, since Truss's class already has the target shape. This
is the key structural difference from `optional`: Deck only ever needs
to build a wrapper when Truss's own contribution is free functions, not
a full class.

Truss's polyfill deliberately doesn't chase perfect fidelity to the
real type -- deletion conditions on the special members are matched
exactly, but conditional triviality and the standard's full two-stage
exception-safe reassignment technique are out of scope, and two smaller
divergences (unconditionally-`explicit` converting constructors, and a
narrower SFINAE guard on the `expected<U,G>` converting constructor)
are disclosed via a `BRIDGE_RIVETS_DIVERGENCE_NOTE` wherever the
polyfill is actually selected. See
[ADR-0010](https://github.com/tokuchan/bridge/blob/master/docs/adr/0010-expected-truss-owns-the-class.md)
for the full fidelity scope and rationale, and
[ADR-0011](https://github.com/tokuchan/bridge/blob/master/docs/adr/0011-warn-on-surprising-facility-divergences.md)
for why these specific gaps get a compiler-visible note rather than
only a code comment.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `bad_expected_access` | class | Thrown by expected&lt;T,E&gt;::value() when accessed without a value; carries a copy of the error that caused the access to fail. Matches std::bad_expected_access&lt;E&gt;. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `expected` | class | Truss's polyfilled expected&lt;T,E&gt;, matching C++23's std::expected&lt;T,E&gt; for standards that predate it. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect` | variable | The canonical unexpect_t instance, passed to select expected's in-place-error constructors. Matches std::unexpect. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect_t` | struct | Tag type selecting expected's error-constructing constructor overloads, mirroring std::in_place_t. Matches std::unexpect_t. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpected` | class | Wraps an error value of type E, matching std::unexpected. Constructed explicitly and passed to expected's converting constructors, or returned directly from a function reporting failure. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
<!-- BRIDGE-DOCS:END -->
