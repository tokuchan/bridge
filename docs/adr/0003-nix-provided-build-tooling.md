# Build Tooling Is Nix-Provided, Not FetchContent

## Context

Tests need Catch2; documentation needs Doxygen. The two common ways to get
them into a CMake build are `FetchContent` (self-contained, fetched over
the network at configure time) or relying on tools already present in the
build environment. This project's build environment is a Nix devShell,
wrapped by `run.sh` — whose own
[ADR-0008](https://github.com/tokuchan/run.sh/blob/master/docs/adr/0008-flake-as-sole-package-specifier.md)
makes `flake.nix` the sole specifier of toolchain packages for exactly this
kind of reproducibility reason.

## Decision

`flake.nix`'s `devShells` (default and every compiler-matrix entry, see
[ADR-0004](0004-compiler-matrix-via-named-devshells.md)) declare `cmake`,
`catch2_3`, and `doxygen` as packages. `CMakeLists.txt` uses
`find_package(Catch2 3 REQUIRED)` and `find_package(Doxygen REQUIRED)` —
no `FetchContent`.

## Consequences

- No network fetch at CMake-configure time; a clean `build/` reconfigures
  instantly as long as the devShell is already warm.
- Catch2/Doxygen versions are pinned by `flake.lock`, not by
  `CMakeLists.txt` — bumping them means updating the flake input, not a
  `FetchContent` tag.
- The project is not buildable outside a Nix devShell without providing
  Catch2/Doxygen some other way. That's an accepted trade — this project
  is already committed to `run.sh` + Nix as its build environment.
