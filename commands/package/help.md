Build DEB/RPM/TBZ2 packages via CPack (see docs/adr/0009-packaging-via-cpack.md).

Runs under the `packaging` devShell by default (`commands/package/conf`),
so plain `./bridge package` works without a `--devshell` flag.

```
./bridge package                # all three generators
./bridge package DEB             # just the .deb
./bridge package --build-dir build-pkg RPM
```
