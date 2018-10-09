#!/bin/bash

set -e

set -v

patch -p0 -i ${CMAKE_SOURCE_DIR}/patches/RTCMTLRenderer.mm.patch
