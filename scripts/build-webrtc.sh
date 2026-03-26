#!/usr/bin/env bash

set -e
set -v

# We want to use system ninja, _NOT_ depot_tools ninja, actually
export PATH="${DEPOT_TOOLS}/python-bin:${PATH}:${DEPOT_TOOLS}"

export TARGETS="webrtc libjingle_peerconnection libc++ libc++abi builtin_video_encoder_factory builtin_video_decoder_factory rtc_internal_video_codecs"

ninja $TARGETS
