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
