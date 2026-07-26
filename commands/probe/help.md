Compile and run a single scratch C++ file against this project's own headers.

Runs inside whatever devshell is active (default, or a named one via
`--devshell`).

Usage:

```
./bridge probe <file.cpp> [--std STANDARD] [-- run-args...]
```

`--std` (default `c++17`) selects the language standard. Everything
after a literal `--` is forwarded as arguments to the compiled binary.
Compiled with `-I include` (this project's own headers) plus
`-Wall -Wextra`; the binary is cleaned up after running.

Examples:

```
./bridge probe /tmp/scratch.cpp
./bridge probe /tmp/scratch.cpp --std c++23
./bridge --devshell clang_20 probe /tmp/scratch.cpp --std c++23
```

For verifying a design idea (a tricky template deduction, a conditionally-
deleted special member, an ABI question) against the real compiler before
committing it to a header -- the empirical-verification step this
project's own development process leans on heavily, rather than trusting
memory of how a construct behaves.
