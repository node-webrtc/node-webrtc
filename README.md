[![NPM](https://nodei.co/npm/wrtc.png?downloads=true&stars=true)](https://nodei.co/npm/wrtc/)

[![Build Status](https://secure.travis-ci.org/js-platform/node-webrtc.png?branch=develop)](http://travis-ci.org/js-platform/node-webrtc) [![Gratipay][wrtc-gratipay-image]][wrtc-gratipay-url]

# Contributing

The best way to get started is to read through the `Getting Started` and `Example` sections before having a look through the open [issues](https://github.com/modeswitch/node-webrtc/issues). Some of the issues are marked as `good first bug`, but feel free to contribute to any of the issues there, or open a new one if the thing you want to work on isn't there yet.

Once you've done some hacking and you'd like to have your work merged, you'll need to make a pull request. If your patch includes code, make sure to check that all the unit tests pass, including any new tests you wrote. Finally, make sure you add yourself to the `AUTHORS` file.

# Getting Started

## Prerequisites

### Debian/Ubuntu

* python2
* git
* pkg-config
* libncurses-devel
* libssl-devel
* libnss-devel
* libexpat-devel

### CentOS/Red Hat
## Install

* python
* git
* perl-ExtUtils-PkgConfig
* openssl-devel
* ncurses-devel
* nss-devel
* expat-devel

### OSX

* XCode

Some of these libraries may be installed by default. Package names for these libraries may differ between distributions.

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

## Troubleshooting

### Error while loading libtinfo.so.5 on Linux

Add a symlink to libncurses:

        sudo ln -s /usr/lib/libncurses.so.5 /usr/lib/libtinfo.so.5

### Some linux distros default to Python3 and the build process fails

This is most common on Arch Linux. Set `python2` as the default for `npm`:

        npm config set python python2

### The node.js package for my distro is too old

Follow the instructions here: https://github.com/joyent/node/wiki/installing-node.js-via-package-manager

# Tests

## Unit tests

Once everything is built, try `npm test` as a sanity check.

## bridge.js
You can run the data channel demo by `node examples/bridge.js` and browsing to `examples/peer.html` in `chrome --enable-data-channels`.

usage:
````
node examples/bridge.js [-h <host>] [-p <port>] [-ws <ws port>]
````
options:
````
-h  host IP for the webserver that will serve the static files (default 127.0.0.1)
-p  host port for the webserver that will serve the static files (default 8080)
-ws port of the Web Socket server (default 9001)
````

If the bridge and peer are on different machines, you can pass the bridge address to the peer by:
````
http://<webserver>/peer.html?<sockertserver:port>
````
By default the bridge will be the same IP as the webserver and will listen on port 9001.

## ping-pong-test.js

The ping-pong example creates two peer connections and sends some data between them.

usage:
````
node examples/ping-pong-test.js
````

[wrtc-gratipay-image]: https://img.shields.io/gratipay/modeswitch.svg?style=flat
[wrtc-gratipay-url]: https://gratipay.com/modeswitch/
