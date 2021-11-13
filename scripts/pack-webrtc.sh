#!/bin/bash

set -e

set -v

mkdir -p lib
ar -r lib/libc++.a `find obj/buildtools/third_party/libc++/libc++ -name '*.o'`

DL=$PWD/../../download
SUBDIRS=(webrtc/{api,rtc_base,modules,system_wrappers,p2p,common_video,common_audio,media,logging,call,pc})

tar Jcf libwebrtc-bin.tar.xz \
    -C $PWD/lib libc++.a \
    -C $PWD/obj/pc libpeerconnection.a \
    -C $PWD/obj libwebrtc.a \
    -C $DL $(cd $DL && find $SUBDIRS -name '*.h') \
    -C $DL/src/buildtools/third_party/libc++/trunk/include .  \
    -C $DL/src/third_party/abseil-cpp $(cd $DL/src/third_party/abseil-cpp && find absl -name '*.h')  \
    -C $DL/src/third_party/libyuv/include $(cd $DL/src/third_party/libyuv/include && find . -name '*.h')
