#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

if [ -z "$PARALLELISM" ]; then
  ninja
else
  ninja -j $PARALLELISM
fi
