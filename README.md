[![NPM](https://nodei.co/npm/wrtc.png?downloads=true&stars=true)](https://nodei.co/npm/wrtc/)

[![OS X/Linus Build Status](https://secure.travis-ci.org/js-platform/node-webrtc.png?branch=develop)](http://travis-ci.org/js-platform/node-webrtc) [![Windows Build status](https://ci.appveyor.com/api/projects/status/iulc84we28o1i7b9?svg=true)](https://ci.appveyor.com/project/markandrus/node-webrtc-7bnua)

# Preamble

This open-source project provides a native module for NodeJS that supports a subset of standards-compliant WebRTC features. Specifically, the PeerConnection and DataChannel APIs. 

__MediaStream APIs are not supported__.

This project relies on precompiled WebRTC binaries provided by [libwebrtc](https://github.com/aisouard/libwebrtc).

# Contributing

The best way to get started is to read through the `Getting Started` and `Example` sections before having a look through the open [issues](https://github.com/modeswitch/node-webrtc/issues). Some of the issues are marked as `good first bug`, but feel free to contribute to any of the issues there, or open a new one if the thing you want to work on isn't there yet.

Once you've done some hacking and you'd like to have your work merged, you'll need to make a pull request. If your patch includes code, make sure to check that all the unit tests pass, including any new tests you wrote. Finally, make sure you add yourself to the `AUTHORS` file.

Whenever possible, prefer making pull requests to opening issues.

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

If you want to work from source:

````
git clone https://github.com/js-platform/node-webrtc.git
cd node-webrtc
npm install
````

# Tests

## Unit tests

Once everything is built, try `npm test` as a sanity check.

## bridge.js

You can run the data channel demo by `node examples/bridge.js` and browsing to [localhost:8080/peer.html](http:localhost:8080/peer.html).

Usage:

````
node examples/bridge.js [-h <host>] [-p <port>] [-ws <ws port>]
````

Options:

````
-h  host IP for the webserver that will serve the static files (default 127.0.0.1)
-p  host port for the webserver that will serve the static files (default 8080)
-ws port of the Web Socket server (default 8080)
````

If the bridge and peer are on different machines, you can pass the bridge address to the peer by:
````
http://<webserver>/peer.html?<sockertserver:port>
````

By default the bridge will be the same IP as the webserver and will listen on port 8080.

## ping-pong-test.js

The ping-pong example creates two peer connections and sends some data between them.

Usage:

````
node examples/ping-pong-test.js
````
