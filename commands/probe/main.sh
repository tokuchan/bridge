#!/bin/sh
set -eu

std="c++17"
if [ "${1:-}" = "--std" ]; then
    std="$2"
    shift 2
fi

src="${1:?usage: ./bridge probe <file.cpp> [--std STANDARD] [-- run-args...]}"
shift

if [ "${1:-}" = "--" ]; then
    shift
fi

out="$(mktemp "${TMPDIR:-/tmp}/bridge-probe.XXXXXX")"
trap 'rm -f "$out"' EXIT

# c++ (not a hardcoded g++/clang++) so this picks up whichever compiler
# the active devshell put on PATH -- confirmed empirically that both the
# default (gcc) and named clang devshells provide a correctly-pointing
# c++, not assumed.
c++ -std="$std" -Wall -Wextra -I "$RUN_ROOT/include" -o "$out" "$src"
"$out" "$@"
