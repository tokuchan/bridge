# Changelog

All notable changes to bridge are documented here.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Versions are CalVer: `YY.MM.MICRO` (see docs/adr/0005-calver-versioning.md).

## [Unreleased]

### Added
- Deck's `format` alias-selection, in a new `include/deck/cpp17/format.hpp`:
  `bridge::format`/`format_to`/`format_to_n`/`formatted_size`/`vformat`
  and their companion types (`format_error`, `format_parse_context`,
  `format_context`, `formatter`, `format_string`, `format_to_n_result`,
  `format_args`) resolve to real `std::format`/etc. once
  `BRIDGE_RIVETS_FEATURES_LIB_FORMAT` confirms native support, or to
  Truss's polyfill otherwise -- same passthrough-or-polyfill shape as
  `deck/cpp17/expected.hpp`. `formatter<T>` is a special case: it's an
  alias template on both branches (C++ can't specialize an alias
  template), so extending formatting for a user type means
  specializing `bridge::truss::formatter<T>` or `std::formatter<T>`
  directly, not `bridge::formatter<T>` itself -- disclosed via a new
  `BRIDGE_RIVETS_DIVERGENCE_NOTE` on the polyfill branch and a new
  `docs/adr/0012` subsection, found during implementation rather than
  anticipated during design.
- Truss's `print`/`println`, in a new `include/truss/cpp17/print.hpp`:
  both `FILE*`-targeting and `ostream`-targeting overload families
  (plus the implicit-stdout no-stream forms), built on this same
  library's `format` unconditionally -- confirmed via benchmark
  (`docs/adr/0012`) that a hypothetical native-`std::format`-backed
  alternative isn't worth the architectural exception it would
  require. Completes Truss's `format`/`print` polyfill; Deck's
  alias-selection for both lands in follow-up commits.
- Truss's top-level format entry points: `format`, `format_to`
  (generic over any output iterator), `format_to_n`, `formatted_size`,
  and `vformat` (with `format_args`/`make_format_args`, a type-erased
  argument pack scoped to `vformat` alone -- this polyfill's scope
  doesn't include `vformat_to`, so unlike real `std::format_args` it
  doesn't need to stay generic over arbitrary output iterators, only
  `std::string`). Also `format_string<Args...>`, a thin wrapper
  implicitly constructible from a string literal, matching real
  `std::format_string`'s shape without its `consteval` compile-time
  validation (C++17 has no `consteval` to do that with -- disclosed in
  `docs/adr/0012`). Runtime dispatch from a format string's arg-id to
  the right argument in a compile-time `Args...` pack uses a fold
  expression over `std::index_sequence`, shared (via the same shape,
  duplicated once for the type-erased path) between `format_to` and
  `vformat`. Verified against real `std::format`/`std::vformat`
  directly: escaped braces, automatic/manual/mixed argument indexing,
  dynamic width/precision, `format_to_n`'s truncation-with-untruncated-
  size contract, and error cases (unmatched braces, out-of-range and
  mixed indices) all cross-checked before writing the Catch2 suite.
- Truss's built-in `formatter<T>` specializations: integer types,
  `bool`, `char`, floating point, pointers (`void*`/`const void*`/
  `nullptr_t`), and the string family (`const char*`/`char*`/
  `std::string`/`std::string_view`), including the C++23 `?` debug
  format for strings and `char`. Built on `std::to_chars` (a real
  C++17 facility) for the actual numeric conversion rather than a
  hand-rolled algorithm. Verified against real `std::format` directly
  throughout development: individual probes for each formatter plus a
  ~200-combination batch differential check (every built-in formatter
  crossed with representative specs), all matching exactly -- this is
  also where the printf-style "precision defaults to 6 when a
  presentation type is given but no explicit precision" rule for
  `f`/`e`/`g` was confirmed (the type-less default instead uses
  `to_chars`' own shortest-round-trip behavior, a real and
  easy-to-miss distinction).
- Truss's `format`/`print` polyfill begins: `format_error`,
  `format_parse_context`, `format_context<OutIt>`, and the disabled-by-
  default `formatter<T>` customization point, in a new
  `include/truss/cpp17/format.hpp`. Includes a hand-written parser for
  the full standard format-spec grammar minus locale (fill-and-align,
  sign, `#`, `0`, width/precision including dynamic `{}`/`{N}`
  references, and the trailing type character) -- cross-checked against
  real `std::format`'s own compile-time diagnostics during development,
  which caught a real parser bug (a leading `0` in the width position
  must still enter width-parsing and get rejected there as "must be
  non-zero", not be skipped over as if no width were present at all).
  Built-in formatters and the top-level `format`/`format_to`/etc. entry
  points land in follow-up commits. See
  `docs/adr/0012-format-print-truss-owns-the-facility.md`.
- New project-wide policy: any Truss-owned facility with a disclosed
  divergence from the real standard facility that could reasonably
  surprise a consumer gets a compiler-visible note, not just
  documentation. `BRIDGE_RIVETS_DIVERGENCE_NOTE("...")`
  (`include/rivets/diagnostics.hpp`) expands to `#pragma message
  "..."`, verified safe under `-Wall -Wextra -Werror` on both GCC (a
  note there, immune to `-Werror` by construction) and Clang
  (categorized as a warning, but didn't trigger `-Werror` in practice
  as tested). See `docs/adr/0011-warn-on-surprising-facility-divergences.md`.
- `rivets/features.hpp`: `__cpp_lib_format` and `__cpp_lib_print`
  Feature Tests (`bridge::rivets::features::lib_format`/`lib_print`),
  first step toward full `std::format`/`std::print`/`std::println`
  support. Confirmed empirically before adding: `std::format` doesn't
  exist before C++20, and while the `<print>` header is includable
  even under `-std=c++20` on GCC, `std::print`/`std::println`
  themselves are hard-rejected until `-std=c++23` -- the two features
  cross their real thresholds at different standards, so they get
  independent Feature Tests rather than one shared gate.

### Fixed
- `bridge::expected`'s polyfill path now surfaces its two
  already-shipped silent divergences from real `std::expected`
  (unconditionally-explicit converting constructors, and the omitted
  `converts-from-any-cvref` SFINAE guards) via
  `BRIDGE_RIVETS_DIVERGENCE_NOTE` when the polyfill branch is active,
  per the new policy above -- retroactively applied per explicit user
  request, rather than left as a gap between the new policy and
  already-shipped work.

## [26.7.1] - 2026-07-26

### Added
- Deck's `bridge::expected<T,E>` (also `bridge::deck::expected<T,E>`):
  a passthrough alias to `std::expected<T,E>` when the ecosystem's
  `__cpp_lib_expected` Feature Test confirms it, or Truss's polyfilled
  `bridge::truss::expected<T,E>` otherwise -- Truss never itself
  passes through, only Deck selects (docs/adr/0010). Unlike
  `bridge::optional` (a wrapper built on Truss's free functions when
  polyfilling), this selection is a plain type alias, since Truss
  already owns a complete class with the right shape. The same
  selection applies to `bridge::unexpected`, `bridge::unexpect_t`/
  `bridge::unexpect`, and `bridge::bad_expected_access` -- confirmed
  necessary by hitting a real compile error first: under passthrough,
  `bridge::expected` *is* `std::expected`, whose constructors expect
  `std::unexpect_t` specifically, not Truss's own (structurally
  identical but distinct) type, so hard-coding those three to Truss's
  polyfill regardless of path would silently break construction the
  moment passthrough activates. Both paths are exercised by the test
  suite (`bridge_deck_tests` at each toolchain's default standard,
  `bridge_deck_expected_cpp23_tests` explicitly at C++23), including a
  differential trait test comparing `bridge::truss::expected` directly
  against `std::expected` in the same C++23 translation unit (possible
  because Truss never passes through, so the two genuinely coexist as
  distinct types there).
- Truss's `expected<void,E>` partial specialization: mirrors the
  primary template member-for-member (constructors, assignment,
  `error()`/`error_or`, `and_then`/`or_else`/`transform`/
  `transform_error`, comparisons, `emplace`, `swap`), minus everything
  that carries a value (`value_or`, and `operator->`/constructing from
  a raw value, which don't apply to `void`). `transform` supports `F`
  returning either `void` (chaining to another `expected<void,E>`) or a
  non-`void` type (producing `expected<U,E>`), using `if constexpr` to
  pick between them. This completes Truss's `expected` polyfill --
  every piece from docs/adr/0010-expected-truss-owns-the-class.md's
  scope is now implemented.

- Truss's `expected<T,E>` comparison operators: against another
  `expected<T2,E2>`, a raw value (both directions), and `unexpected<G>`
  (both directions). Also fixes a gap in the prior `unexpected<E>`
  commit: `std::unexpected`/`std::expected` only define `operator==`,
  relying on C++20's automatic `!=` rewriting from it -- unavailable
  under this header's C++17 floor, so `!=` genuinely failed to compile
  before this commit despite `==` working. `operator!=` is now defined
  explicitly everywhere `==` is, for both `unexpected<E>` and
  `expected<T,E>`, matching passthrough usability.
- Truss's `expected<T,E>` monadic operations: `and_then`, `or_else`,
  `transform`, `transform_error` (full value-category fidelity,
  matching `std::expected`'s member-function shape rather than
  `optional`'s free-function-plus-wrapper split, since Truss already
  owns the whole class here), and `error_or` -- absent from the
  original C++23 proposal (P0323R12 explicitly excluded it) but present
  under the same `__cpp_lib_expected` value as the rest of the type on
  every ecosystem this project's compiler matrix covers, confirmed by
  direct compile probe earlier this session. `transform`'s `F`
  returning `void` (which would produce `expected<void,E>`) isn't
  supported yet -- that needs the `expected<void,E>` partial
  specialization landing in a follow-up commit.
- Truss's `expected<T,E>` core: constructors (default, value, `unexpect`,
  `unexpected<G>`, copy, move, and a converting constructor from
  `expected<U,G>`), copy/move assignment plus assignment from a raw
  value or `unexpected<G>`, `has_value()`/`operator bool`, `operator->`/
  `operator*`, `value()` (throwing) and `error()` (precondition-UB,
  matching `std::expected`), `value_or`, `emplace`, and `swap` (member
  and ADL, including across differing alternatives). Implemented via a
  layered-base idiom (one layer per conditionally-deleted special
  member) so the exact deletion conditions `std::expected` uses match —
  verified with `static_assert`s against hand-picked throwing-move and
  non-copyable types, and against a standalone probe before wiring it
  into the real header.
- Truss's `expected<T,E>` polyfill begins: `bridge::truss::unexpected<E>`,
  `bridge::truss::bad_expected_access<E>`, and the `unexpect_t`/`unexpect`
  tag, in a new `include/truss/cpp17/expected.hpp`. Unlike `optional`,
  `std::expected` doesn't exist before C++23 at all, so Truss owns a
  complete from-scratch class here rather than free functions on an
  existing STL type — a second refinement of the Truss/Deck boundary.
  `bridge::truss::expected<T,E>` (landing in follow-up commits) will be
  unconditionally this polyfill regardless of standard/toolchain; Deck
  alone selects between it and the real `std::expected`. See
  `docs/adr/0010-expected-truss-owns-the-class.md`. `CONTEXT.md`'s Truss
  and Deck entries updated accordingly.
- `rivets/features.hpp`: `__cpp_lib_expected` Feature Test
  (`bridge::rivets::features::lib_expected`), first step toward full
  `std::expected` support. Unlike `__cpp_lib_optional`, this one
  legitimately reports `0` on pre-C++23 toolchains — `std::expected`
  doesn't exist at all before C++23, confirmed by direct compiler
  probe (GCC/Clang both fail to find `<expected>` under
  `-std=c++17`/`-std=c++20`) rather than assumed.

### Fixed
- Truss's `expected<T,E>::transform` rejected a `void`-returning `F`
  with a `static_assert`, deferred until `expected<void,E>` existed to
  chain to (added in the following commit) -- but the primary template
  was never actually updated once it landed. `std::expected::transform`
  has always supported `F` returning `void` (producing
  `expected<void,E>`), so code compiling against passthrough
  (`bridge::expected<int,E>{5}.transform([](int){})`) failed to compile
  against the polyfill -- the exact "behaviorally indistinguishable"
  violation docs/adr/0008 exists to prevent, confirmed by compiling the
  discriminating case under both the default (polyfill) and
  `-std=c++23` (passthrough) targets before fixing it, not assumed. All
  four `transform` overloads now branch on `is_void_v<U>` via `if
  constexpr`, matching `expected<void,E>`'s own `transform`.
- `rivets/features.hpp` checked `__cpp_lib_*` macros without first
  including `<version>` (the SD-6-designed header that exposes every
  library feature-test macro without pulling in each one's full
  implementation). For `__cpp_lib_optional` this was masked by
  consumers always including `<optional>` first regardless; for
  `__cpp_lib_expected` it will be a real chicken-and-egg bug the moment
  a consumer checks the Feature Test to decide whether it's even safe
  to `#include <expected>` (which doesn't exist at all pre-C++23) --
  without `<version>`, the macro would be invisible and the check would
  always report `0`, meaning a passthrough path gated on it could never
  actually activate on any toolchain, regardless of real support.
- Truss's `expected<T,E>::emplace` was unconditionally available for
  any `Args` constructible into `T`; `std::expected::emplace` requires
  `is_nothrow_constructible_v<T, Args...>` specifically, since
  destroy-then-construct is only exception-safe when the construct step
  can't throw. Confirmed by hitting a real compile error against the
  real `std::expected` (`expected<std::string, int>::emplace("hello")`
  -- `std::string(const char*)` can throw via allocation) once Deck's
  passthrough selection made that comparison possible, not assumed; the
  tests that relied on the missing constraint now move from an existing
  `std::string` instead.
- Doxygen's handling of class-template partial specializations
  conflates member lookup with the primary template: documenting
  `expected<void,E>`'s members caused Doxygen to resolve the primary
  template's `value_or` with `T` substituted as `void` and flag its
  `@return` doc as invalid, confirmed by isolating the exact cause
  (removing the specialization made the unrelated-looking error
  disappear) rather than assumed. Worked around with `\cond`/`\endcond`
  around the specialization's body, same posture as the earlier
  `bad_expected_access` recursive-class-relation false positive.
- `unexpected<E>` only got `operator==` in the commit that introduced
  it; `std::unexpected` relies on C++20's automatic `!=` rewriting from
  `==`, unavailable under this header's C++17 floor, so `!=` genuinely
  failed to compile despite `==` working. Added `operator!=` explicitly
  for both `unexpected<E>` and `expected<T,E>`, matching passthrough
  usability.

## [26.7.0] - 2026-07-26

### Added
- Initial project scaffold: `rivets` (detection macros), `truss` (C++17
  polyfills), `deck` (higher-level utilities) as header-only libraries under
  a shared `include/` tree, each with a smoke-test header and Catch2 test
  proving the namespace/build/test pipeline end to end.
- Namespace and export scheme: `bridge::detail::<lib>::<standard>::<header>`
  → curated `::exports` → `bridge::exports::<lib>::<header>` (inline
  standard/header segments) → hand-picked `bridge::<lib>::` promotion. See
  `docs/adr/0001-namespace-and-export-scheme.md`.
- CMake build: single root `CMakeLists.txt`, one `INTERFACE` target per
  library (`bridge::rivets`, `bridge::truss`, `bridge::deck`), Catch2 3 and
  Doxygen sourced from the Nix devShell rather than `FetchContent`. See
  `docs/adr/0003-nix-provided-build-tooling.md`.
- `BRIDGE_WITH_BOOST` CMake option (`AUTO`/`ON`/`OFF`): Boost is optional
  and feature-detected, never a hard dependency. See
  `docs/adr/0002-boost-optional-feature-detected.md`.
- Doxygen documentation target (`./bridge docs`), configured to warn on any
  undocumented entity — full documentation coverage is enforced, not just
  aspirational.
- Compiler test matrix via named Nix devShells (`gcc13`/`14`/`15`,
  `clang_18`–`21`) and `scripts/test-matrix.sh`. Required extending the
  `run.sh` submodule with named devShell selection
  (`--devshell`/`RUN_DEVSHELL_NAME`/`devshell=`); see
  `docs/adr/0004-compiler-matrix-via-named-devshells.md`.
- `run.sh` pulled in as the `./bridge` command (git submodule at `.run`).
- CalVer versioning (`YY.MM.MICRO`); see
  `docs/adr/0005-calver-versioning.md`.
- Rivets detector naming calculus: a Detector per Entity (seeded constant
  + `gt`/`ge`/`lt`/`le`/`eq` as both constexpr functions and `#if`-usable
  function-like macros), plus `BRIDGE_RIVETS_DEFINE_DETECTOR`/
  `_DEFINE_DETECTOR_RANGE` to generate Named Detectors on demand. Real
  Detectors for GCC, Clang, and the C++ standard (extending
  `rivets/standard.hpp`); Boost, MSVC, libstdc++, and libc++ stubbed with
  their intended shape documented but not yet implemented. See
  `docs/adr/0006-detector-naming-calculus.md`.
- Doxygen `MACRO_EXPANSION`/`EXPAND_AS_DEFINED` for the detector generator
  macros, so Named Detectors (which only exist via macro expansion) are
  visible to the documentation-coverage gate instead of silently invisible.
- `rivets/features.hpp`: Feature Test wrapping (`__cpp_lib_optional`), a new
  Rivets concept distinct from Detector — a 1:1 wrap of an SD-6 feature-test
  macro rather than a computed version range. See
  `docs/adr/0007-feature-test-wrapping.md`.
- Truss's first real feature: `bridge::truss::and_then`/`or_else`/`transform`
  free functions on plain `std::optional<T>`, full value-category fidelity
  (matching C++23's monadic `std::optional` methods), usable from C++17.
  `CONTEXT.md`'s Truss entry updated: Truss provides free-function
  primitives only, never a wrapper type with member methods (that's Deck's
  job going forward).
- Deck's first STL-shaped wrapper type: `bridge::optional<T>` (also
  `bridge::deck::optional<T>`), a passthrough alias to `std::optional<T>`
  when the detected ecosystem's Feature Test confirms native monadic
  support, or a wrapper built on Truss's free functions otherwise — no
  detectable difference between the two paths. Both paths are exercised by
  the test suite (`bridge_deck_tests` at each toolchain's default standard,
  `bridge_deck_optional_cpp23_tests` explicitly at C++23). See
  `docs/adr/0008-best-effort-head-standard.md`. `CONTEXT.md`'s Deck entry
  updated accordingly.
- Packaging: `install()` rules for headers + `bridge::rivets`/`truss`/`deck`,
  a `find_package(bridge CONFIG)` package config
  (`cmake/bridgeConfig.cmake.in`), and `cmake/FetchBridge.cmake`'s
  `bridge_fetch()` for consumers who want bridge without installing a
  package first. CPack builds `.deb` (`libbridge-dev`), `.rpm`
  (`bridge-devel`), and `.tar.bz2`, driven by a new `./bridge package`
  command running under a dedicated `packaging` Nix devShell selected
  automatically via its own default (`commands/package/conf`). A Nix
  package output (`nix build .#bridge`) is a fourth distribution channel.
  See `docs/adr/0009-packaging-via-cpack.md`.

### Fixed
- `include/{rivets,truss,deck}/CMakeLists.txt` used `CMAKE_SOURCE_DIR`
  (always the *outermost* project's root) instead of
  `CMAKE_CURRENT_SOURCE_DIR` (bridge's own root regardless of nesting) for
  the include path — broke header resolution the moment bridge was
  consumed via `FetchContent`/`add_subdirectory`. `BRIDGE_BUILD_TESTS` and
  `install()`/CPack now default off when bridge isn't the top-level
  project, so a `FetchContent` consumer doesn't get bridge's own test
  suite or install rules as a side effect of using the library.

### Changed
- `run.sh` submodule bumped for its new per-command default devShell
  (`devshell =` in `commands/<cmd>/conf`, root→leaf inherited like
  `dispatch`), used by `commands/package/conf`.
