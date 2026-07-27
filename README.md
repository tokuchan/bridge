# bridge

Bridge the gaps between C++ standards, compilers, and STL implementations.
Header-only libraries, usable from C++17 onward:

- **rivets** — macros for detecting language standard, compiler, STL, and
  Boost versions.
- **truss** — polyfills of modern C++ features for older standards,
  compilers, or STLs that lack them, built on Rivets' detection.
- **deck** — higher-level utilities built on top of Truss.

## Documentation

The full generated site — a cppreference-inspired index, one page per
conceptual facility (not just per header), and full symbol
documentation — is browsable at
<https://tokuchan.github.io/bridge/>. See
[docs/adr/0014](docs/adr/0014-documentation-site-architecture.md) for
how it's built. To build it locally instead: `./bridge docs` (output
under `build/docs/html`).

## Quick Start

```sh
git clone --recurse-submodules https://github.com/tokuchan/bridge
cd bridge
./bridge --help
```
