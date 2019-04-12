/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <cstddef>

#include <node-addon-api/napi.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/node/napi_async_object_wrap_with_loop.h"

namespace node_webrtc {

class RTCAudioSink
  : public napi::AsyncObjectWrapWithLoop<RTCAudioSink>
  , public webrtc::AudioTrackSinkInterface {
 public:
  explicit RTCAudioSink(const Napi::CallbackInfo&);

  static void Init(Napi::Env, Napi::Object);

  void OnData(
      const void* audio_data,
      int bits_per_sample,
      int sample_rate,
      size_t number_of_channels,
      size_t number_of_frames) override;

  static Napi::FunctionReference& constructor();

 protected:
  void Stop() override;

 private:
  Napi::Value GetStopped(const Napi::CallbackInfo&);

  Napi::Value JsStop(const Napi::CallbackInfo&);

  bool _stopped = false;
  rtc::scoped_refptr<webrtc::AudioTrackInterface> _track;
};

}  // namespace node_webrtc
