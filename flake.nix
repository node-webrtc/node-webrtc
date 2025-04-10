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
        pkgs = nixpkgs.legacyPackages.${system};
        lib = nixpkgs.lib;
        is-darwin = lib.strings.hasSuffix "darwin" system;
      in
      {
        devShells.default =
          let
            apple-sdk = if is-darwin then pkgs.apple-sdk_12 else null;
            clang = pkgs.llvmPackages_14.clang;
            clang-tools = pkgs.llvmPackages_14.clang-tools;
          in
          pkgs.mkShell {
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
              ++ (if is-darwin then [ apple-sdk ] else [ ]);
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
          };
      }
    );
}
