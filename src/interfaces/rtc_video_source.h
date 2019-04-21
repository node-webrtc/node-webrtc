/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <absl/types/optional.h>
#include <node-addon-api/napi.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/media/base/adapted_video_track_source.h>

#include "src/dictionaries/node_webrtc/rtc_video_source_init.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"

namespace webrtc { class VideoFrame; }

namespace node_webrtc {

class RTCVideoTrackSource : public rtc::AdaptedVideoTrackSource {
 public:
  RTCVideoTrackSource()
    : rtc::AdaptedVideoTrackSource(), _is_screencast(false) {}

  RTCVideoTrackSource(const bool is_screencast, const absl::optional<bool> needs_denoising)
    : rtc::AdaptedVideoTrackSource(), _is_screencast(is_screencast), _needs_denoising(needs_denoising) {}

  ~RTCVideoTrackSource() override {
    PeerConnectionFactory::Release();
    _factory = nullptr;
  }

  SourceState state() const override {
    return webrtc::MediaSourceInterface::SourceState::kLive;
  }

  bool remote() const override {
    return false;
  }

  bool is_screencast() const override {
    return _is_screencast;
  }

  absl::optional<bool> needs_denoising() const override {
    return _needs_denoising;
  }

  void PushFrame(const webrtc::VideoFrame& frame) {
    this->OnFrame(frame);
  }

 private:
  PeerConnectionFactory* _factory = PeerConnectionFactory::GetOrCreateDefault();
  const bool _is_screencast;
  const absl::optional<bool> _needs_denoising;
};

class RTCVideoSource
  : public Napi::ObjectWrap<RTCVideoSource> {
 public:
  explicit RTCVideoSource(const Napi::CallbackInfo&);

  static void Init(Napi::Env, Napi::Object);

 private:
  static Napi::FunctionReference& constructor();

  Napi::Value New(const Napi::CallbackInfo&);

  Napi::Value GetIsScreencast(const Napi::CallbackInfo&);
  Napi::Value GetNeedsDenoising(const Napi::CallbackInfo&);

  Napi::Value CreateTrack(const Napi::CallbackInfo&);
  Napi::Value OnFrame(const Napi::CallbackInfo&);

  rtc::scoped_refptr<RTCVideoTrackSource> _source;
};

}  // namespace node_webrtc
