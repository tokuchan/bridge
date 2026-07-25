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
    in
    {
      devShells.${system} = {
        default = mkDevShell pkgs.stdenv;
      } // gccShells // clangShells;
    };
}
