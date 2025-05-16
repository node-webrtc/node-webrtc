{
  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-24.11";
    flake-utils.url = "github:numtide/flake-utils";
  };
  outputs =
    {
      self,
      nixpkgs,
      flake-utils,
    }:
    flake-utils.lib.eachDefaultSystem (
      system:
      let
        pkgs = import nixpkgs { inherit system; };
      in
      {
        devShells.default = (import ./shell.nix) { inherit pkgs system; };

        packages = {
          aarch64-linux-gcc = pkgs.pkgsCross.aarch64-multiplatform.buildPackages.gcc10;
        };
      }
    );
}
