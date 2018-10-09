#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

cd ${SOURCE_DIR}

gn gen ${BINARY_DIR} "--args=${GN_GEN_ARGS}"
