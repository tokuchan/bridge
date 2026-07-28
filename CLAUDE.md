# bridge project — Claude Code instructions

## Running tests (gate)

**Always** run the test suite via:

```sh
./bridge test
```

This builds and runs the full Catch2 suite inside the project's Nix
devShell. It is the gate command for the `/commit` skill — run it before
every commit.

## ADR compliance review (mandatory before every commit)

Before committing any code change, read every ADR in `docs/adr/` and check
the change against each decision recorded there. A change that contradicts
an existing ADR must not be committed silently — either adjust the change
to comply, or raise the conflict with the user and record a new ADR
(marked as superseding the old one) before proceeding.

## STE writing convention for the manual

Facility pages (`docs/pages/<name>.md`) and every promoted symbol's
`@brief`/`@details` Doxygen comment follow ASD-STE100 (Simplified
Technical English): short sentences, one fact per sentence, active
voice, no parenthetical asides. See
[ADR-0018](docs/adr/0018-ste100-writing-standard.md) for the full
decision and its scope. `docs/pages/ste-glossary.md` is the approved
technical-name list for bridge/C++/programming terms — use a term only
with the meaning recorded there, and add a new entry when a facility
conversion needs a term the glossary doesn't have yet.

ADRs, `CONTEXT.md`, and `CHANGELOG.md` are explicitly out of scope —
keep their existing rationale-heavy, citation-style voice.

## Commit atomically, for `git bisect`

When executing a plan (design session or otherwise), don't batch every
file touched into one commit at the end. Split the work into the
smallest units that are each independently meaningful, and commit each
one as it's finished — not just at the very end of the whole task.

**Every commit must build and pass the full gate on its own.** Before
committing a unit, make the working tree match *only* that unit (e.g.
`git stash push --keep-index -u` to set aside anything else in progress,
`git stash pop` after committing) and run `./bridge test` against that
isolated state — not the combined state of everything currently on
disk. A commit that only compiles once a *later* commit lands breaks
`git bisect`, which is the entire point of splitting the work up.

A reasonable unit is usually "one library's worth of one feature" (e.g.
Rivets' half of a change, then Truss's, then Deck's, when a feature
spans the stack) — small enough to isolate a regression to, large
enough to be a coherent, reviewable idea. Cross-cutting docs
(`CONTEXT.md`, `CHANGELOG.md`) split the same way: each commit carries
only the glossary/changelog lines that specific commit's code actually
motivates, not the whole session's worth at once.

`git-atomic-commit` (a global script, not part of this repo) automates
the stash/gate/commit/restore dance above: stage the unit's files, then
pipe the commit message into `git-atomic-commit --gate './bridge test'
--gate './bridge docs'`.

## Before cutting a release

Run the full compiler matrix (`./scripts/test-matrix.sh`), not just the
default devShell or a couple of spot-checked `--devshell` runs — it
builds and tests against every devShell in `flake.nix`
(gcc13–15, clang_18–21) and is the actual gate a release should clear,
not a substitute for per-commit velocity during a design session.

**Tag format:** prefix the git tag with `v` (e.g. `v26.7.1`, not
`26.7.1`) — GitHub's release tooling expects this.

**CHANGELOG title:** when moving a release's entries out of
`[Unreleased]` into a dated section, give that section a title/header
alongside the version — GitHub wants a release title distinct from the
description, and it should come from the CHANGELOG rather than be
invented at tag time.
