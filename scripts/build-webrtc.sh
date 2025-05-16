#!/usr/bin/env bash

set -e
set -v

# We want to use system ninja, _NOT_ depot_tools ninja, actually
export PATH="${DEPOT_TOOLS}/python-bin:${PATH}:${DEPOT_TOOLS}"

export TARGETS="webrtc libjingle_peerconnection libc++ libc++abi"

ninja $TARGETS
