#!/bin/bash

platform=$(uname -s | tr '[A-Z]' '[a-z]')
arch=${TARGET_ARCH:-$(uname -m | tr '[A-Z]' '[a-z]')}

set -e

set -v

tar xf libwebrtc-${platform}-${arch}.tar.gz

