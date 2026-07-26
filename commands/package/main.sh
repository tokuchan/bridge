#!/bin/sh
set -eu

build_dir="build"
if [ "${1:-}" = "--build-dir" ]; then
    build_dir="$2"
    shift 2
fi

cmake -S "$RUN_ROOT" -B "$RUN_ROOT/$build_dir" -DBRIDGE_BUILD_TESTS=OFF
cmake --build "$RUN_ROOT/$build_dir"

cd "$RUN_ROOT/$build_dir"
if [ "$#" -gt 0 ]; then
    exec cpack -G "$1"
else
    exec cpack -G "TBZ2;DEB;RPM"
fi
