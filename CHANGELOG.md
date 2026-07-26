# Changelog

All notable changes to bridge are documented here.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Versions are CalVer: `YY.MM.MICRO` (see docs/adr/0005-calver-versioning.md).

## [Unreleased]

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
