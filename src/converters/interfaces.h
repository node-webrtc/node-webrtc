/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <v8.h>

#include "src/converters.h"
#include "src/functional/validation.h"

namespace rtc {

template <class T> class scoped_refptr;

}  // namespace rtc

namespace webrtc {

class AudioTrackInterface;
class VideoTrackInterface;

}  // namespace webrtc

namespace node_webrtc {

class MediaStream;
class MediaStreamTrack;
class RTCRtpReceiver;
class RTCRtpSender;
class RTCRtpTransceiver;

DECLARE_TO_AND_FROM_JS(MediaStream*)
DECLARE_TO_AND_FROM_JS(MediaStreamTrack*)
DECLARE_TO_JS(RTCRtpReceiver*)
DECLARE_TO_AND_FROM_JS(RTCRtpSender*)
DECLARE_TO_JS(RTCRtpTransceiver*)

DECLARE_CONVERTER(MediaStreamTrack*, rtc::scoped_refptr<webrtc::AudioTrackInterface>)
DECLARE_CONVERTER(MediaStreamTrack*, rtc::scoped_refptr<webrtc::VideoTrackInterface>)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::AudioTrackInterface>)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::VideoTrackInterface>)

}  // namespace node_webrtc
