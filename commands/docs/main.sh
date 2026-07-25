#!/bin/sh
set -eu

build_dir="build"
if [ "${1:-}" = "--build-dir" ]; then
    build_dir="$2"
    shift 2
fi

cmake -S "$RUN_ROOT" -B "$RUN_ROOT/$build_dir" -DBRIDGE_BUILD_DOCS=ON
cmake --build "$RUN_ROOT/$build_dir" --target docs
