# Build from Source

## Prerequisites

### macOS/Linux

On macOS and Linux, a reproducible build environment is provided in the form of a Nix dev shell. You can install Nix using the [Lix installer](https://lix.systems/install/), then activate the build environment using `nix develop`.

### Windows

node-webrtc uses [cmake-js](https://github.com/cmake-js/cmake-js) to build
from source. When building from source, in addition to the prerequisites
required by cmake-js, you will need

- Git
- Ninja
- CMake 3.15 or newer
- Microsoft Visual Studio 2022 or newer, with the Clang toolchain installed
- Check the [additional prerequisites listed by WebRTC](https://webrtc.github.io/webrtc-org/native-code/development/prerequisite-sw/) - although their install is automated by the CMake scripts provided

## Building

Once you have the prerequisites, just clone the repository, run `npm install` to install the Javascript dependencies, and `npm run build` to build node-webrtc for your host platform.

```
git clone https://github.com/node-webrtc/node-webrtc.git
cd node-webrtc
npm install
nix develop # macOS/Linux only
npm run build
```

### Subsequent Builds

Subsequent builds can be triggered with `cmake`, e.g. on MacOS:

```
cmake --build build-darwin-arm64
```

You can pass either `--debug` or `--release` to build a debug or release build
of node-webrtc (and the underlying WebRTC library). Refer to the CMake
documentation for additional command line options.

### Cross-Compiling

The supported cross-compilation directions are:

- MacOS arm64 ➡️ MacOS x64
- MacOS x64 ➡️ ️MacOS arm64
- Linux x64 ➡️ Linux arm64

To run e.g. that that cross-compilation:

1. Set `TARGET_ARCH` to "arm64"
2. Re-run `npm run build`

### Debug Builds

1. Set `DEBUG=1`
2. Re-run `npm run build`

This is only fully-supported on macOS/Linux; Windows support is a bit dicey thanks to CMake.

To run a release build after doing a debug build, `unset DEBUG`.

## Other Notes

### Linux

On Linux, we compile libwebrtc's sources with libwebrtc's Clang toolchain and libwebrtc's sysroot. We statically link against its libc++ and libc++abi. This is a little dangerous (due to ABI concerns) but we match Clang major versions so it's probably OK?

### macOS

On macOS, we compile libwebrtc's sources with our own clang toolchain and our own sysroot. We statically link against its libc++ and libc++abi.

### Windows

We use the Clang toolchain and the Ninja generator on Windows in order to have
similar support for the `clangd` language server and `compile_commands.json`;
Visual Studio proper has not been tested.

To fix the error `Filename too long`, when downloading libwebrtc, use
(optionally with `--global` or `--system` switches to set for more than just
this project):

```
git config core.longpaths true
```

Creating symbolic links with MKLINK is used by the build script but is disabled
for non-Administrative users by default with a local security policy. On
Windows 10, fix this with Run (Windows-R) then `gpedit.msc`. Edit key "Local
Computer Policy -> Windows Settings -> Security Settings -> Local Policies ->
User Rights Assignment -> Create Symbolic Links" and add your user name. Log
out and in to change the policy. Note the [associated security
vunerability](https://docs.microsoft.com/en-us/windows/security/threat-protection/security-policy-settings/create-symbolic-links#vulnerability).

The Windows SDK debugging tools should be installed. One way to achieve this is
to [Download the Windows Driver
Kit](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk).

# Test

## Unit & Integration Tests

Once everything is built, run

```
npm test
```

> [!WARNING]
> If testing a release build, make sure to remove the debug build in `build-{os}-{arch}/Debug/wrtc.node` first! This takes precedence in module resolution.
