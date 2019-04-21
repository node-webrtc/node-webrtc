/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <node-addon-api/napi.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/api/video/video_sink_interface.h>

#include "src/node/async_object_wrap_with_loop.h"

namespace webrtc { class VideoFrame; }

namespace node_webrtc {

class RTCVideoSink
  : public AsyncObjectWrapWithLoop<RTCVideoSink>
  , public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  explicit RTCVideoSink(const Napi::CallbackInfo&);

  static void Init(Napi::Env, Napi::Object);

  void OnFrame(const webrtc::VideoFrame& frame) override;

  static Napi::FunctionReference& constructor();

 protected:
  void Stop() override;

 private:
  Napi::Value GetStopped(const Napi::CallbackInfo&);

  Napi::Value JsStop(const Napi::CallbackInfo&);

  bool _stopped = false;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> _track;
};

}  // namespace node_webrtc
