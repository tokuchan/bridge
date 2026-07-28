# Compiler Test Matrix Uses Named Nix devShells, Not Separate Docker Images

## Context

Rivets' whole job is detecting compiler/standard/STL/Boost differences, and
Truss's job is picking the right polyfill based on what Rivets detects.
Neither claim can be trusted without actually building and testing against
more than one compiler. The project already runs everything through
`run.sh`, which wraps a single Nix flake devShell per project — so the
choice was between standing up a second containerization mechanism (one
Dockerfile per compiler) or defining multiple named devShells inside the
one `flake.nix` already in use.

## Decision

`flake.nix` defines `devShells.default` plus one named devShell per
`(compiler, version)` pair — currently `gcc13`, `gcc14`, `gcc15`, `clang_18`,
`clang_19`, `clang_20`, `clang_21`. Every devShell, matrix or default, gets
the same `cmake`/`catch2_3`/`doxygen` tooling; only the `stdenv` (and
therefore the compiler) changes. All of them still run inside `run.sh`'s
existing container isolation — nothing new to build or maintain outside the
one flake.

This range is not a guess: `gcc9Stdenv`–`gcc12Stdenv` and
`llvmPackages_12`–`llvmPackages_17` were checked directly against the
pinned `nixpkgs-unstable` snapshot and throw `has been removed, as it is
unmaintained and obsolete` on evaluation. The matrix reflects what's
actually live, not what was originally assumed when scoping this decision.

`run.sh` itself had no way to select a non-default devShell before this —
it always resolved `devShells.default`. Extending it was in scope; see its
own [ADR-0021](https://github.com/tokuchan/run.sh/blob/master/docs/adr/0021-named-devshell-selection.md)
for the `--devshell`/`RUN_DEVSHELL_NAME`/`devshell=` setting this relies on.

`scripts/test-matrix.sh` drives the loop across devShells. It's a plain
host-side script, not a `run.sh` command — devshell selection happens
before the container ever launches, so the loop that switches between
devshells has to live on the host, outside any single containerized
invocation. Each devshell gets its own CMake build directory
(`build-<name>`) so switching compilers doesn't reuse a stale CMake cache
from a previous compiler.

## Consequences

- Adding a compiler to the matrix means adding one `flake.nix` entry —
  no new `Dockerfile`, no new CI image to build and push.
- The matrix is exactly as wide as nixpkgs' current snapshot allows; it
  will drift over time as nixpkgs drops old compiler versions and adds new
  ones, and should be re-verified (not re-assumed) when that happens.
- **Known gap: `gcc13`'s linker fails on every test binary**, a real
  binary-compatibility issue with sharing one `catch2_3` across the whole
  matrix (line 19 above), not a bug in bridge's own code. `catch2_3` is a
  single nixpkgs-prebuilt package -- the identical store path is reused by
  every devShell, compiled once against nixpkgs' ambient default stdenv
  (GCC 14/15-class on the pinned snapshot). That compiler's cold-path
  splitting for exception cleanup emits calls to `__cxa_call_terminate`, a
  libstdc++ runtime symbol -- confirmed via `nm -D` directly against each
  devShell's own `libstdc++.so` to be present in GCC 14 and GCC 15, but
  genuinely absent from GCC 13's. The final link of any Catch2-linked test
  binary under `gcc13` therefore fails with "undefined reference to
  `__cxa_call_terminate`", regardless of what the test itself does --
  reproduced against a commit with zero jthread/threading-related changes,
  ruling out anything project-specific. Recorded here rather than fixed:
  the real fix (rebuilding `catch2_3` per-devShell stdenv, or dropping
  `gcc13` from the matrix) is a real trade-off (build time vs. support
  floor) deserving its own decision, not a silent workaround.
