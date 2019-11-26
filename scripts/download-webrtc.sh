#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

gclient config --unmanaged --spec 'solutions=[{"name":"src","url":"https://webrtc.googlesource.com/src.git"}]'

gclient sync --shallow --no-history --nohooks --with_branch_heads -r ${WEBRTC_REVISION} -R

python src/tools/clang/scripts/update.py

rm -f webrtc

ln -s src webrtc
