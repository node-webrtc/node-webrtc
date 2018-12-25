#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

if [ -z "$PARALLELISM" ]; then
  ninja webrtc libjingle_peerconnection
else
  ninja webrtc libjingle_peerconnection -j $PARALLELISM
fi
