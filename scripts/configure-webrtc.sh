#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

cd ${SOURCE_DIR}

if [ "$TARGET_ARCH" == "arm" ]; then
    python build/linux/sysroot_scripts/install-sysroot.py --arch=arm
else
    python build/linux/sysroot_scripts/install-sysroot.py --arch=amd64
fi

gn gen ${BINARY_DIR} "--args=${GN_GEN_ARGS}"
