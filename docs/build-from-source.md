# Build from Source

## Prerequisites

node-webrtc uses [node-cmake](https://github.com/cjntaylor/node-cmake) to build
from source. When building from source, in addition to the prerequisites
required by node-cmake, you will need

* Git
* CMake 3.12 or newer
* GCC 5.4 or newer (Linux)
* Xcode 9 or newer (macOS)
* Microsoft Visual Studio 2019 (Windows)
* Check the [additional prerequisites listed by WebRTC](https://webrtc.github.io/webrtc-org/native-code/development/prerequisite-sw/) - although their install is automated by the CMake scripts provided


## Install

Once you have the prerequisites, clone the repository, set the `SKIP_DOWNLOAD`
environment variable to "true", and run `npm install`. Just like when
installing prebuilt binaries, you can set the `TARGET_ARCH` environment
variable to "arm" or "arm64" to build for armv7l or arm64, respectively. Linux
and macOS users can also set the `DEBUG` environment variable for debug builds.

```
git clone https://github.com/node-webrtc/node-webrtc.git
cd node-webrtc
SKIP_DOWNLOAD=true npm install
```

Note: Use `$SKIP_DOWNLOAD = 'true'; npm install` on Windows Powershell.

## Subsequent Builds

Subsequent builds can be triggered with `ncmake`:

```
./node_modules/.bin/ncmake configure
./node_modules/.bin/ncmake build
```

You can pass either `--debug` or `--release` to build a debug or release build
of node-webrtc (and the underlying WebRTC library). Refer to
[node-cmake](https://github.com/cjntaylor/node-cmake) for additional
command-line options to `ncmake`.

## Other Notes

### Linux

On Linux, we statically link libc++ and libc++abi. Also, although we compile
WebRTC sources with Clang (downloaded as part of WebRTC's build process), we
compile node-webrtc sources with GCC 5.4 or newer.

#### armv7l

In order to cross-compile for armv7l on Linux,

1. Set `TARGET_ARCH` to "arm".
2. Install the appropriate toolchain, and set `ARM_TOOLS_PATH`.
3. On Ubuntu, you may also need g++-arm-linux-gnueabihf.

```
wget https://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/arm-linux-gnueabihf/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabihf.tar.xz
tar xf gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabihf.tar.xz
SKIP_DOWNLOAD=true TARGET_ARCH=arm ARM_TOOLS_PATH=$(pwd)/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabihf npm install
```

#### arm64

In order to cross-compile for arm64 on Linux,

1. Set `TARGET_ARCH` to "arm64".
2. Install the appropriate toolchain, and set `ARM_TOOLS_PATH`.
3. On Ubuntu, you may also need g++-aarch64-linux-gnu.

```
wget https://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
tar xf gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
SKIP_DOWNLOAD=true TARGET_ARCH=arm64 ARM_TOOLS_PATH=$(pwd)/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu npm install
```

### macOS

On macOS, we statically link libc++ and libc++abi. Also, we compile WebRTC
sources with the version of Clang downloaded as part of WebRTC's build process,
but we compile node-webrtc sources using the system Clang.

### Windows

On Windows, we do not compile WebRTC sources with Clang. This is disabled by
passing `is_clang=false` to `gn gen`.

To fix error `Filename too long`, use (optionally with `--global` or `--system` switches to set for more than just this project):

```
git config core.longpaths true
```

Creating symbolic links with MKLINK is used by the build script but is disabled for non-Administrative users by default with a local security policy. On Windows 10, fix this with Run (Windows-R) then `gpedit.msc`. Edit key "Local Computer Policy -> Windows Settings -> Security Settings -> Local Policies -> User Rights Assignment -> Create Symbolic Links" and add your user name. Log out and in to change the policy. Note the [associated security vunerability](https://docs.microsoft.com/en-us/windows/security/threat-protection/security-policy-settings/create-symbolic-links#vulnerability).

The Windows SDK debugging tools should be installed. One way to achieve this is to [Download the Windows Driver Kit](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk).

# Test

## Unit & Integration Tests

Once everything is built, run

```
npm test
```

## Web Platform Tests

[web-platform-tests/wpt](https://github.com/web-platform-tests/wpt) defines a suite of WebRTC tests. node-webrtc borrows a technique from [jsdom/jsdom](https://github.com/jsdom/jsdom) to run these tests in Node.js. Run the tests with

```
npm run wpt:test
```

## Browser Tests

These tests are run by Circle CI to ensure node-webrtc remains compatible with
the latest versions of Chrome and Firefox.

```
npm run test:browsers
```

## Electron Test

```
npm run test:electron
```
