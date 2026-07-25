Build (if needed) and run the Catch2 test suite via ctest.

`--build-dir <dir>` (default `build`) selects the build directory. Remaining
arguments are forwarded to ctest, e.g.:

```
./bridge test -R truss
./bridge --devshell clang_20 test --build-dir build-clang_20
```
