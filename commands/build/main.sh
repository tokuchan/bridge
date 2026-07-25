#!/bin/sh
set -eu

# The container inherits the host's XDG_RUNTIME_DIR (e.g. /run/user/1000),
# which doesn't exist inside the container filesystem. Catch2's
# catch_discover_tests POST_BUILD step needs a writable one to list tests.
[ -d "${XDG_RUNTIME_DIR:-}" ] || export XDG_RUNTIME_DIR=/tmp

build_dir="build"
if [ "${1:-}" = "--build-dir" ]; then
    build_dir="$2"
    shift 2
fi

cmake -S "$RUN_ROOT" -B "$RUN_ROOT/$build_dir" "$@"
cmake --build "$RUN_ROOT/$build_dir"
