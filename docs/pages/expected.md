\page page_expected Expected

<!-- BRIDGE-DOCS:BEGIN headers -->
- `include/truss/cpp17/expected.hpp`
- `include/deck/cpp17/expected.hpp`
<!-- BRIDGE-DOCS:END -->

TODO: narrative prose for this facility.

<!-- BRIDGE-DOCS:BEGIN symbols -->
| Symbol | Kind | Brief | cppreference |
|---|---|---|---|
| `bad_expected_access` | class | Thrown by expected&lt;T,E&gt;::value() when accessed without a value; carries a copy of the error that caused the access to fail. Matches std::bad_expected_access&lt;E&gt;. Forward-declared here so the void specialization below (its common base) can reference it; defined for real further down. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `expected` | class | Truss's polyfilled expected&lt;T,E&gt;, matching C++23's std::expected&lt;T,E&gt; for standards that predate it. See the file-level docs and docs/adr/0010-expected-truss-owns-the- class.md for the fidelity scope this implements: deletion conditions on the special members are matched exactly; conditional triviality and the standard's full two-stage exception-safe reassignment are explicitly out of scope. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect` | variable | The canonical unexpect_t instance, passed to select expected's in-place-error constructors. Matches std::unexpect. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpect_t` | struct | Tag type selecting expected's error-constructing constructor overloads, mirroring std::in_place_t. Matches std::unexpect_t. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
| `unexpected` | class | Wraps an error value of type E, matching std::unexpected. Constructed explicitly and passed to expected's converting constructors, or returned directly from a function reporting failure. | [https://en.cppreference.com/w/cpp/utility/expected](https://en.cppreference.com/w/cpp/utility/expected) |
<!-- BRIDGE-DOCS:END -->
