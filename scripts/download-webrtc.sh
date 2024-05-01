#!/bin/bash

PROJ=$(realpath $(dirname $0)/..)

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

gclient config --unmanaged --spec 'solutions=[{"name":"src","url":"https://webrtc.googlesource.com/src.git"}]'

gclient sync --shallow --no-history --nohooks --with_branch_heads -r ${WEBRTC_REVISION} -R

python src/tools/clang/scripts/update.py

( cd src/base && git apply $PROJ/etc/g++-base.patch )

rm -f webrtc

ln -s src webrtc
