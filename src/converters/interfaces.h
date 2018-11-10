/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_CONVERTERS_INTERFACES_H_
#define SRC_CONVERTERS_INTERFACES_H_

#include <nan.h>

#include "src/converters.h"

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

}  // namespace node_webrtc

#endif  // SRC_CONVERTERS_INTERFACES_H_
