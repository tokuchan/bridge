#!/bin/sh
set -eu

build_dir="build"
if [ "${1:-}" = "--build-dir" ]; then
    build_dir="$2"
    shift 2
fi

cmake -S "$RUN_ROOT" -B "$RUN_ROOT/$build_dir" -DBRIDGE_BUILD_DOCS=ON

# Two-pass build (docs/adr/0014): the generated "symbols" block in
# each facility page is sourced from *this run's own* Doxygen XML
# output, but that same run also needs to render the just-updated page
# content into HTML. Pass 1 produces XML from whatever the pages
# currently say; scripts/docs-pages.py regenerates the pages from that
# XML (also failing here if any header isn't assigned to a facility in
# docs/pages/registry.yaml); pass 2 re-renders HTML from the
# now-current pages.
cmake --build "$RUN_ROOT/$build_dir" --target docs
python3 "$RUN_ROOT/scripts/docs-pages.py" generate
cmake --build "$RUN_ROOT/$build_dir" --target docs
