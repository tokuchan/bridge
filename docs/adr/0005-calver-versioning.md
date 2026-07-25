# Versioning Uses CalVer (YY.MM.MICRO)

## Context

Bridge needs a version scheme for `CHANGELOG.md`, `CMakeLists.txt`'s
`project(VERSION ...)`, and Doxygen's `PROJECT_NUMBER`. The sister `run.sh`
project (see its `CHANGELOG.md`) already uses a CalVer-style scheme — bare
`YYYY-MM-DD` release headers — but that shape doesn't parse as a numeric
version triple, which tools that consume `project(VERSION ...)` (CMake
itself, package managers, `find_package` version constraints) expect.

## Decision

Versions are `YY.MM.MICRO` — two-digit year, month (no zero-padding), and a
`MICRO` counter that increments for additional releases within the same
calendar month and resets to `0` at the start of the next month. For
example: `26.7.0` (first release cut in July 2026), `26.7.1` (a second
release the same month), `26.8.0` (first release in August 2026).

No zero-padding on month, so `26.7.0` not `26.07.0` — a leading zero on a
numeric identifier is invalid in strict SemVer parsing, and this scheme is
meant to be parsed by ordinary version-triple tooling without special
CalVer-aware handling.

`CHANGELOG.md` follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/):
unreleased work accumulates under `## [Unreleased]`; cutting a release means
moving that section under a `## [YY.MM.MICRO]` header and bumping
`project(bridge VERSION ...)` in `CMakeLists.txt` in the same commit — the
same "explicit release-cut" discipline `run.sh`'s own `CONTRIBUTING.md`
uses (`chore(release): cut <version>`).

## Consequences

- `CMakeLists.txt`'s `PROJECT_VERSION` is the single source of truth for the
  current version; Doxygen's `PROJECT_NUMBER` is derived from it via
  `configure_file`, not maintained separately.
- Until the first release is cut, `PROJECT_VERSION` stays `0.0.0` — there is
  no "current in-progress version" concept, matching `run.sh`'s own
  precedent of only stamping a version at the moment of an actual release.
- The version doesn't encode which C++ standards or compilers were tested
  against a given release; that's `CHANGELOG.md` prose, not the version
  number's job.
