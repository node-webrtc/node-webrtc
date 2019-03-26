/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/events.h"

#include "src/rtcvideosink.h"

using node_webrtc::OnFrameEvent;
using node_webrtc::RTCVideoSink;

void OnFrameEvent::Dispatch(RTCVideoSink& sink) {
  sink.HandleOnFrameEvent(*this);
}
