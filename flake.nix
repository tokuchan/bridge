{
  description = "bridge dev environment";

  inputs.nixpkgs.url = "github:NixOS/nixpkgs/nixpkgs-unstable";

  outputs = { self, nixpkgs }:
    let
      # Change x86_64-linux to aarch64-linux for ARM hosts.
      system = "x86_64-linux";
      pkgs   = nixpkgs.legacyPackages.${system};

      # Compiler matrix (docs/adr/0004-compiler-matrix-via-named-devshells.md).
      # Only stdenvs that are actually live in the pinned nixpkgs-unstable
      # snapshot; older aliases (gcc9-12Stdenv, llvmPackages_12-17) have been
      # dropped upstream as unmaintained and throw on evaluation.
      gccVersions = [ 13 14 15 ];
      clangVersions = [ 18 19 20 21 ];

      # Every devShell — default and matrix alike — gets the same build/test/
      # docs tooling; only the compiler (stdenv) changes.
      commonPackages = with pkgs; [
        bash
        coreutils
        git
        cmake
        catch2_3
        doxygen
        boost
        # run:packages — managed by 'run --add / --remove'; do not delete this line
      ];

      mkDevShell = stdenv: pkgs.mkShell.override { inherit stdenv; } {
        packages = commonPackages;
      };

      gccShells = builtins.listToAttrs (map (v: {
        name = "gcc${toString v}";
        value = mkDevShell pkgs."gcc${toString v}Stdenv";
      }) gccVersions);

      clangShells = builtins.listToAttrs (map (v: {
        name = "clang_${toString v}";
        value = mkDevShell pkgs."llvmPackages_${toString v}".stdenv;
      }) clangVersions);

      # dpkg/rpm are only needed for packaging (docs/adr/0009-packaging-
      # via-cpack.md), not for everyday build/test/docs work -- kept out
      # of commonPackages so the compiler-matrix devShells and default
      # stay lean. cpack itself is already bundled with the cmake package.
      bridgePackage = pkgs.stdenv.mkDerivation {
        pname = "bridge";
        # Matches PROJECT_VERSION until a release is actually cut
        # (docs/adr/0005-calver-versioning.md).
        version = "0.0.0";
        src = self;
        nativeBuildInputs = [ pkgs.cmake ];
        cmakeFlags = [
          "-DBRIDGE_BUILD_TESTS=OFF"
          "-DBRIDGE_WITH_BOOST=OFF"
        ];
      };
    in
    {
      devShells.${system} = {
        default = mkDevShell pkgs.stdenv;
        packaging = pkgs.mkShell {
          packages = commonPackages ++ (with pkgs; [ dpkg rpm ]);
        };
      } // gccShells // clangShells;

      packages.${system} = {
        bridge = bridgePackage;
        default = bridgePackage;
      };
    };
}
