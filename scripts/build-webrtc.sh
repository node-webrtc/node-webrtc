#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

export TARGETS="webrtc libjingle_peerconnection pc:create_pc_factory"
if [[ "$TARGET_ARCH" == arm* ]]; then
  export TARGETS="$TARGETS pc:peerconnection libc++ libc++abi"
fi

if [ -z "$PARALLELISM" ]; then
  ninja $TARGETS
else
  ninja $TARGETS -j $PARALLELISM
fi
