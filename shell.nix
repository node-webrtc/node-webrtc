{
  pkgs ? import <nixpkgs> { },
  system ? builtins.currentSystem,
}:
let
  lib = pkgs.lib;
  is-darwin = lib.strings.hasSuffix "darwin" system;

  env =
    pkgs:
    (
      let
        apple-sdk = if is-darwin then pkgs.apple-sdk_13 else null;
        llvm =
          pkgs:
          pkgs.llvmPackages_17.override {
            inherit apple-sdk;
          };
        clang = (llvm pkgs).clang;
      in
      {
        stdenv = (llvm pkgs).stdenv;
        nativeBuildInputs =
          (with pkgs; [
            cmake
            ninja
            nodejs_24
            pkg-config
            zlib
            # For stripping binaries for release
            patchelf
          ])
          ++ (with (llvm pkgs); [
            clang
            clang-tools
            libllvm
          ])
          ++ (lib.optionals is-darwin [
            apple-sdk
            pkgs.xcbuild
          ])
          ++ (lib.optionals (!is-darwin) [
            (llvm pkgs.pkgsCross.aarch64-multiplatform.buildPackages).clang
          ]);
        # Build variables based on documentation from https://github.com/timniederhausen/gn-build/blob/01c96fd9981b111a3a028356284968acd77fa435/README.md
        shellHook = ''
          cat <<EOF > nix.gni
          is_clang=true
          use_lld=false
          clang_use_chrome_plugins=false
        ''
        + (lib.optionalString is-darwin ''
          clang_base_path="${clang}"
          mac_sdk_path="${apple-sdk}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
        '')
        + ''
          EOF

          cat <<EOF > .lazy.lua
          -- Override the version of clang used for clangd
          return {
            {
              "neovim/nvim-lspconfig",
              opts = {
                servers = {
                  clangd = {
                    cmd = { "clangd", "--query-driver=${clang}/bin/clang++" },
                  },
                },
              },
            },
          }
          EOF
        '';
      }
    );

in
if is-darwin then
  pkgs.mkShell {
    inherit (env pkgs) nativeBuildInputs shellHook;
  }
else
  (pkgs.buildFHSEnv.override { inherit (env pkgs) stdenv; } {
    name = "node-webrtc-env";
    targetPkgs = pkgs: (env pkgs).nativeBuildInputs;
    profile = (env pkgs).shellHook;
    runScript = "bash";
  }).env
