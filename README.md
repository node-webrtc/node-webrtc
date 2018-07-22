<p align="center">
  <img height="120px" src="https://upload.wikimedia.org/wikipedia/commons/d/d9/Node.js_logo.svg" />&nbsp;&nbsp;&nbsp;&nbsp;
  <img height="120px" src="https://webrtc.org/assets/images/webrtc-logo-vert-retro-dist.svg" />
</p>

[![NPM](https://img.shields.io/npm/v/wrtc.svg)](https://www.npmjs.com/package/wrtc) [![macOS/Linux Build Status](https://secure.travis-ci.org/js-platform/node-webrtc.svg?branch=develop)](http://travis-ci.org/js-platform/node-webrtc) [![Windows Build status](https://ci.appveyor.com/api/projects/status/iulc84we28o1i7b9?svg=true)](https://ci.appveyor.com/project/markandrus/node-webrtc-7bnua)

node-webrtc provides Node.js bindings to [WebRTC M60](https://github.com/mayeut/libwebrtc/releases/tag/v1.1.1). You can write Node.js applications that use RTCDataChannels with it. **Some MediaStream APIs are supported now!.**

|         | x86 | x64 | arm | arm64 |
|:------- |:--- |:--- |:--- |:----- |
| Linux   |     | ✔︎   |     |       |
| macOS   |     | ✔︎   |     |       |
| Windows |     | ✔︎   |     |       |

# Getting Started

## Prerequisites

This library will attempt to download pre-compiled binaries for your particular
platform using [node-pre-gyp](https://github.com/mapbox/node-pre-gyp); however,
if binaries are unavailable, it will fallback to building from source. In this
case, the prerequisites for building from source are the same as
[node-cmake](https://github.com/cjntaylor/node-cmake). Refer to node-cmake for the
particular prerequisites for your platform.

## Install

The easiest way to install is via npm:

````
npm install wrtc
````

If you want to work from source, run

````
git clone https://github.com/js-platform/node-webrtc.git
cd node-webrtc
npm install
````

## Build

If you would like to build libwebrtc _and_ node-webrtc from source (you must if you're using arm), run

```
git clone https://github.com/aisouard/libwebrtc
cd libwebrtc
mkdir out
cd out
cmake -DWEBRTC_BRANCH_HEAD=refs/branch-heads/57 ..
make
# wait for a while
cd <path of node-webrtc>
mkdir -p third_party/webrtc
cp -r <path of libwebrtc>/out/lib third_party/webrtc
cp -r <path of libwebrtc>/out/include third_party/webrtc
export SKIP_DOWNLOAD=true
export CC=gcc-4.8
export CXX=g++-4.8
npm run install
```

_**Note:** These instructions need to be updated for WebRTC M60._

# Tests

## node-webrtc Tests

Once everything is built, run

```
npm test
```

## Web Platform Tests

[w3c/web-platform-tests](https://github.com/w3c/web-platform-tests) defines a suite of WebRTC tests. node-webrtc borrows a technique from [jdom/jsdom](https://github.com/jsdom/jsdom) to run these tests in Node.js. Run the tests with

```
npm run wpt:test
```

## MediaStream Loopback Example

This example demonstrates relaying MediaStreamTracks through node-webrtc. Run
the example with

```
node examples/loopback.server.js
```

Then navigate to [http://localhost:8080/loopback.client.html](http://localhost:8080/loopback.client.html).
You should be prompted for your microphone and webcam. Once granted, the browser
negotiates an RTCPeerConnection with the server, and the server relays the
browser's MediaStreamTracks. Finally, these are displayed in a &lt;video&gt;
element in the browser.

# Contributing

The best way to get started is to read through the `Getting Started` and `Example` sections before having a look through the open [issues](https://github.com/modeswitch/node-webrtc/issues). Some of the issues are marked as `good first bug`, but feel free to contribute to any of the issues there, or open a new one if the thing you want to work on isn't there yet.

Once you've done some hacking and you'd like to have your work merged, you'll need to make a pull request. If your patch includes code, make sure to check that all the unit tests pass, including any new tests you wrote. Finally, make sure you add yourself to the `AUTHORS` file.

Whenever possible, prefer making pull requests to opening issues.
