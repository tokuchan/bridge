#!/bin/sh
set -eu

# See commands/build/main.sh for why this is needed.
[ -d "${XDG_RUNTIME_DIR:-}" ] || export XDG_RUNTIME_DIR=/tmp

build_dir="build"
if [ "${1:-}" = "--build-dir" ]; then
    build_dir="$2"
    shift 2
fi

[ -d "$RUN_ROOT/$build_dir" ] || cmake -S "$RUN_ROOT" -B "$RUN_ROOT/$build_dir"
cmake --build "$RUN_ROOT/$build_dir"
ctest --test-dir "$RUN_ROOT/$build_dir" --output-on-failure "$@"
