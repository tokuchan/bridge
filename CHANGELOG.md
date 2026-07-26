# Changelog

All notable changes to bridge are documented here.
Format: [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
Versions are CalVer: `YY.MM.MICRO` (see docs/adr/0005-calver-versioning.md).

## [Unreleased]

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
