Configure and build the CMake project.

`--build-dir <dir>` (default `build`) selects the build directory — used by
`scripts/test-matrix.sh` to keep each compiler's CMake cache separate.
Remaining arguments are forwarded to the CMake configure step, e.g.:

```
./bridge build -DBRIDGE_WITH_BOOST=OFF
./bridge --devshell gcc14 build --build-dir build-gcc14
```
