#!/usr/bin/env bash

# A simple utility script for getting in the same context as configure-webrtc.sh, but interactively

scripts="$(dirname -- "${BASH_SOURCE[0]}")"
build_folder="$(cd "${scripts}" && node -e "console.log(require('./build-vars.js').buildFolder)")"
external="$(realpath "${build_folder}/external")"
DEPOT_TOOLS="${external}/depot_tools/src"
LIBWEBRTC="${external}/libwebrtc/download/src"

cd "${LIBWEBRTC}"
PATH="$DEPOT_TOOLS:${DEPOT_TOOLS}/python-bin:$PATH" /usr/bin/env zsh -i
