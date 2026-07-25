#!/bin/sh
# Runs the test suite under every compiler devShell defined in flake.nix
# (see docs/adr/0004-compiler-matrix-via-named-devshells.md).
#
# This is a plain host-side script, not a run.sh command: devshell selection
# happens in run.sh before the container launches, so the loop across
# devshells has to live outside any single containerized invocation.
set -eu

cd "$(dirname "$0")/.."

DEVSHELLS="gcc13 gcc14 gcc15 clang_18 clang_19 clang_20 clang_21"

failed=""
for shell in $DEVSHELLS; do
    echo "==> $shell"
    build_dir="build-$shell"
    if ! ./bridge --devshell "$shell" build --build-dir "$build_dir"; then
        failed="$failed $shell(build)"
        continue
    fi
    if ! ./bridge --devshell "$shell" test --build-dir "$build_dir"; then
        failed="$failed $shell(test)"
    fi
done

if [ -n "$failed" ]; then
    echo "FAILED:$failed" >&2
    exit 1
fi

echo "All devshells passed: $DEVSHELLS"
