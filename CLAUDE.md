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
