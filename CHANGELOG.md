# Changelog

All notable changes to bridge are documented here.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Versions are CalVer: `YY.MM.MICRO` (see docs/adr/0005-calver-versioning.md).

## [Unreleased]

### Added
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
