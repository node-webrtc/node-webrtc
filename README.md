<p align="center">
  <img height="120px" src="https://upload.wikimedia.org/wikipedia/commons/d/d9/Node.js_logo.svg" />&nbsp;&nbsp;&nbsp;&nbsp;
  <img height="120px" src="https://webrtc.org/assets/images/webrtc-logo-vert-retro-dist.svg" />
</p>

[![NPM](https://img.shields.io/npm/v/wrtc.svg)](https://www.npmjs.com/package/wrtc) [![macOS/Linux Build Status](https://circleci.com/gh/node-webrtc/node-webrtc/tree/develop.svg?style=shield)](https://circleci.com/gh/node-webrtc/node-webrtc) [![Windows Build status](https://ci.appveyor.com/api/projects/status/iulc84we28o1i7b9?svg=true)](https://ci.appveyor.com/project/markandrus/node-webrtc-7bnua)

node-webrtc provides Node.js bindings to [WebRTC M73](https://chromium.googlesource.com/external/webrtc/+/branch-heads/73). You can write Node.js applications that use RTCDataChannels with it. **Some MediaStream APIs are supported.**

|         | x86 | x64 | armv6l | armv7l | arm64 |
|:------- |:--- |:--- |:------ |:------ |:----- |
| Linux   | -   | ✔︎   | ✘      | ✔︎      | ✔︎     |
| macOS   | -   | ✔︎   | -      | -      | -     |
| Windows | ✘   | ✔︎   | -      | -      | -     |

# Getting Started

## Prerequisites

This library will attempt to download pre-compiled binaries for your particular
platform using [node-pre-gyp](https://github.com/mapbox/node-pre-gyp); however,
if binaries are unavailable, it will fallback to building from source using
[node-cmake](https://github.com/cjntaylor/node-cmake). When building from
source, in addition to the prerequisites required by node-cmake, you will need

* Git
* CMake 3.12 or newer
* GCC 5.4 or newer (Linux)
* Xcode 9 or newer (macOS)
* Microsoft Visual Studio 2017 (Windows)
* Any [additional prerequisites listed by WebRTC](https://webrtc.org/native-code/development/prerequisite-sw)

## Install

The easiest way to install is via npm:

```
npm install wrtc
```

If you want to work from source, run

```
git clone https://github.com/js-platform/node-webrtc.git
cd node-webrtc
npm install
```

Depending on what you checkout, `npm install` will either download a
pre-compiled binary or attempt to build from source. Set `SKIP_DOWNLOAD=true` to
always build from source. See below for more information on building from
source.

## Build

If you would like to build node-webrtc from source, run

```
./node_modules/.bin/ncmake rebuild
```

You can pass either `--debug` or `--release` to build a debug or release build
of node-webrtc (and the underlying WebRTC library). Refer to
[node-cmake](https://github.com/cjntaylor/node-cmake) for additional
command-line options to `ncmake`.

### Other Notes

#### Linux

On Linux, we statically link libc++ and libc++abi. Also, although we compile
WebRTC sources with Clang (downloaded as part of WebRTC's build process), we
compile node-webrtc sources with GCC 5.4 or newer.

##### armv7l

In order to cross-compile for armv7l on Linux,

1. Set `TARGET_ARCH` to "arm".
2. Install the appropriate toolchain, and set `ARM_TOOLS_PATH`.
3. On Ubuntu, you may also need g++-arm-linux-gnueabihf.

```
wget https://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/arm-linux-gnueabihf/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabihf.tar.xz
tar xf gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabihf.tar.xz
SKIP_DOWNLOAD=true TARGET_ARCH=arm ARM_TOOLS_PATH=$(pwd)/gcc-linaro-7.3.1-2018.05-x86_64_arm-linux-gnueabihf npm install
```

##### arm64

In order to cross-compile for arm64 on Linux,

1. Set `TARGET_ARCH` to "arm64".
2. Install the appropriate toolchain, and set `ARM_TOOLS_PATH`.
3. On Ubuntu, you may also need g++-aarch64-linux-gnu.

```
wget https://releases.linaro.org/components/toolchain/binaries/7.3-2018.05/aarch64-linux-gnu/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
tar xf gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu.tar.xz
SKIP_DOWNLOAD=true TARGET_ARCH=arm64 ARM_TOOLS_PATH=$(pwd)/gcc-linaro-7.3.1-2018.05-x86_64_aarch64-linux-gnu npm install
```

#### macOS

On macOS, we compile WebRTC sources with the version of Clang downloaded as part
of WebRTC's build process, but we compile node-webrtc sources using the system
Clang.

#### Windows

On Windows, we do not compile WebRTC sources with Clang. This is disabled by
passing `is_clang=false` to `gn gen`.

# Tests

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

## node-webrtc Examples

See [example/](example) for a node-webrtc example application demonstrating
RTCDataChannels, RTCRtpTransceivers, and more.
