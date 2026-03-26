#!/usr/bin/env bash

set -e

set -v

export PATH=$DEPOT_TOOLS:${DEPOT_TOOLS}/python-bin:$PATH

cd ${SOURCE_DIR}

case "$(uname -s)" in
  Linux*)
    if [ "$TARGET_ARCH" == "arm" ]; then
      vpython3 build/linux/sysroot_scripts/install-sysroot.py --arch=arm
    elif [ "$TARGET_ARCH" == "arm64" ]; then
      vpython3 build/linux/sysroot_scripts/install-sysroot.py --arch=arm64
    else
      vpython3 build/linux/sysroot_scripts/install-sysroot.py --arch=amd64
    fi
esac

# NOTE(mroberts): Running hooks generates this file, but running hooks also
# takes too long in CI; so do this manually.
(cd build/util && vpython3 lastchange.py -o LASTCHANGE)

gn gen ${BINARY_DIR} "--args=${GN_GEN_ARGS}"
