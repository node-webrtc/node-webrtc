#!/bin/bash

set -e

set -v

export PATH=$DEPOT_TOOLS:$PATH

gclient config --unmanaged --spec 'solutions=[{"name":"src","url":"https://webrtc.googlesource.com/src.git"}]'

gclient sync --shallow --no-history --nohooks --with_branch_heads -r ${WEBRTC_REVISION} -R

if [ "${PLATFORM}" = darwin ]; then
  download_from_google_storage --no_resume --platform=${PLATFORM} --no_auth --bucket chromium-gn -s src/buildtools/mac/gn.sha1
else
  download_from_google_storage --no_resume --platform=${PLATFORM} --no_auth --bucket chromium-gn -s src/buildtools/linux64/gn.sha1
fi

python src/tools/clang/scripts/update.py

rm -f webrtc

ln -s src webrtc
