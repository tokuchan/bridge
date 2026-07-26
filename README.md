# bridge

Bridge the gaps between C++ standards, compilers, and STL implementations.
Header-only libraries, usable from C++17 onward:

- **rivets** — macros for detecting language standard, compiler, STL, and
  Boost versions.
- **truss** — polyfills of modern C++ features for older standards,
  compilers, or STLs that lack them, built on Rivets' detection.
- **deck** — higher-level utilities built on top of Truss.

## Quick Start

```sh
git clone --recurse-submodules https://github.com/tokuchan/bridge
cd bridge
./bridge --help
```
