# Packaging via CPack, Nix-Provisioned Tooling

## Context

`bridge` is header-only — "packaging" means distributing headers plus a
CMake package config (no compiled binary), in a form other projects can
consume via a system package manager, a plain archive, Nix, or CMake's
own `FetchContent`. There was no `install()` rule at all before this;
this is the last piece before cutting a first official release.

## Decision

**CPack**, driven entirely from `CMakeLists.txt`, generates every
package format — no hand-written `debian/rules` or `.spec` files.
Matches the project's existing full commitment to CMake, and avoids two
parallel, hand-maintained packaging descriptions that would drift from
whatever `install()` actually declares.

**Packaging tools are Nix-provisioned**, matching
[ADR-0003](0003-nix-provided-build-tooling.md): `dpkg`, `rpm`, and
`cpack` (bundled with the existing `cmake` package) live in a new,
dedicated `devShells.packaging` — not `commonPackages`, so the
compiler-matrix devShells and everyday `default` stay lean. Selected via
the existing `--devshell` mechanism
([ADR-0004](0004-compiler-matrix-via-named-devshells.md)), with
`commands/package/conf` declaring `devshell = packaging` as that
command's default (`run.sh`'s own ADR-0022) so `./bridge package` just
works without repeating `--devshell` every time.

**A Nix package output** (`packages.default` in `flake.nix`) is a fourth
distribution channel alongside DEB/RPM/TBZ2 — trivial for a header-only
library, since the derivation is just the same `cmake --install` step
Nix's generic CMake builder already knows how to run.

**CMake consumer support, two pieces**:

- A proper package config (`BridgeConfig.cmake` +
  `BridgeConfigVersion.cmake`, via `CMakePackageConfigHelpers`) installed
  by every package format: `find_package(bridge CONFIG REQUIRED)` →
  `bridge::rivets`, `bridge::truss`, `bridge::deck` targets. No
  per-library `COMPONENTS` — all three ship together, one version, one
  package; there's no scenario yet where a consumer would want one
  without the others.
- `cmake/FetchBridge.cmake`: a `bridge_fetch()` function wrapping
  `FetchContent`, for consumers who want bridge without installing a
  package first. This lives in the repo itself (and is also installed,
  for discoverability from an existing install) rather than *only*
  inside the installed tree, since its whole purpose is being available
  *before* bridge is installed. This does not conflict with ADR-0003 —
  that ADR governs how bridge builds *itself*, not how other projects
  may choose to consume it.

**Package naming** avoids colliding with unrelated real-world packages
named plain "bridge": `libbridge-dev` for the DEB (Debian's `lib*-dev`
convention for header-only C++ libraries), `bridge-devel` for the RPM
(Fedora's analogous convention), `bridge` for the TBZ2 and Nix outputs.

## Platform scope

DEB, RPM, TBZ2, and the Nix package now. All four build from the same
platform-neutral CMake install tree (plain headers + `INTERFACE`
targets, nothing Linux-specific), so a plain CPack `ZIP` generator would
work unmodified on macOS/Windows whenever someone actually builds there.
macOS (Homebrew/`.dmg`), Windows (NSIS/WIX), iOS (Xcode/SPM consumption),
and additional Nix systems (`aarch64-darwin`, `x86_64-darwin`) are
explicit future work — this project can't verify platforms it has no
hardware or CI for, so it isn't guessing at their packaging specifics
now.

## Consequences

- Package version numbers read `0.0.0` until an actual release is cut
  (per [ADR-0005](0005-calver-versioning.md)) — expected while verifying
  the packaging infrastructure itself, not a bug.
- Adding a platform later means adding a CPack generator (or a Nix
  system) and verifying it on real hardware/CI — not redesigning the
  install tree, which is already platform-neutral.
