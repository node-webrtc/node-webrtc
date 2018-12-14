#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

cd ${SOURCE_DIR}

if [ "$TARGET_ARCH" == "arm" ]; then
  python build/linux/sysroot_scripts/install-sysroot.py --arch=arm
elif [ "$TARGET_ARCH" == "arm64" ]; then
  python build/linux/sysroot_scripts/install-sysroot.py --arch=arm64
else
  python build/linux/sysroot_scripts/install-sysroot.py --arch=amd64
fi
gn gen ${BINARY_DIR} "--args=${GN_GEN_ARGS}"
