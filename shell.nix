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
        apple-sdk = if is-darwin then pkgs.apple-sdk_12 else null;
        llvm = pkgs.llvmPackages_14;
        clang = llvm.clang.overrideAttrs {
          apple-sdk = apple-sdk;
        };
        clang-tools = llvm.clang-tools.overrideAttrs {
          apple-sdk = apple-sdk;
        };
      in
      {
        stdenv = llvm.stdenv;
        nativeBuildInputs =
          (with pkgs; [
            cmake
            ninja
            nodejs_20
            pkg-config
            zlib
          ])
          ++ [
            clang
            clang-tools
          ]
          ++ (
            if is-darwin then
              [
                apple-sdk
                pkgs.xcbuild
              ]
            else
              [ ]
          );
        # Build variables based on documentation from https://github.com/timniederhausen/gn-build/blob/01c96fd9981b111a3a028356284968acd77fa435/README.md
        shellHook =
          ''
            cat <<EOF > nix.gni
            is_clang=true
            use_lld=false
            clang_base_path="${clang}"
            clang_use_chrome_plugins=false
          ''
          + (
            if is-darwin then
              ''
                mac_sdk_path="${apple-sdk}/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk"
              ''
            else
              ""
          )
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

  mkShell =
    pkgs:
    (
      let
        e = env pkgs;
      in
      if is-darwin then
        pkgs.mkShell {
          inherit (e) nativeBuildInputs shellHook;
        }
      else
        (pkgs.buildFHSEnv.override { inherit (e) stdenv; } {
          name = "node-webrtc-env";
          targetPkgs = pkgs: (env pkgs).nativeBuildInputs;
          # TODO: shellHook somewher
          runScript = "bash";
        }).env
    );
in
mkShell pkgs
