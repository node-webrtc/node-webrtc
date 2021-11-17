#!/bin/bash

set -e

set -v

DL=$PWD/../../download
SUBDIRS=$(echo webrtc/{api,rtc_base,modules,system_wrappers,p2p,common_video,common_audio,media,logging,call,pc})

if [ -d obj/buildtools/third_party/libc++ ] ; then
  mkdir -p lib
  ar -r lib/libc++.a `find obj/buildtools/third_party/libc++{,abi} -name '*.o'`
  LIBCXX=(
    -C $PWD/lib libc++.a
    -C $DL/src/buildtools/third_party/libc++/trunk/include .
    -C $DL/src/buildtools/third_party/libc++ __config_site
  )
fi

tar Jcf libwebrtc-bin.tar.xz \
    ${LIBCXX} \
    -C $PWD/obj/pc libpeerconnection.a \
    -C $PWD/obj libwebrtc.a \
    -C $DL $(cd $DL && find $SUBDIRS -name '*.h') \
    -C $DL/src/third_party/abseil-cpp $(cd $DL/src/third_party/abseil-cpp && find absl -name '*.h')  \
    -C $DL/src/third_party/libyuv/include $(cd $DL/src/third_party/libyuv/include && find . -name '*.h')