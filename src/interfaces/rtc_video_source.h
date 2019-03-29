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
#include <nan.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/media/base/adapted_video_track_source.h>
#include <v8.h>

#include "src/converters/dictionaries.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"

namespace webrtc { class VideoFrame; }

namespace node_webrtc {

class RTCVideoTrackSource : public rtc::AdaptedVideoTrackSource {
 public:
  RTCVideoTrackSource()
    : rtc::AdaptedVideoTrackSource(), _is_screencast(false) {}

  RTCVideoTrackSource(const bool is_screencast, const absl::optional<bool> needs_denoising)
    : rtc::AdaptedVideoTrackSource(), _is_screencast(is_screencast), _needs_denoising(needs_denoising) {}

  ~RTCVideoTrackSource() override = default;

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
  const std::shared_ptr<PeerConnectionFactory> _factory = PeerConnectionFactory::GetOrCreateDefault();
  const bool _is_screencast;
  const absl::optional<bool> _needs_denoising;
};

class RTCVideoSource
  : public Nan::ObjectWrap {
 public:
  RTCVideoSource();

  explicit RTCVideoSource(RTCVideoSourceInit);

  ~RTCVideoSource() override = default;

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);

 private:
  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetIsScreencast);
  static NAN_GETTER(GetNeedsDenoising);

  static NAN_METHOD(CreateTrack);
  static NAN_METHOD(OnFrame);

  rtc::scoped_refptr<RTCVideoTrackSource> _source;
};

}  // namespace node_webrtc
