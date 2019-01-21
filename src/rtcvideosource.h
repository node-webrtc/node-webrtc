/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCVIDEOSOURCE_H_
#define SRC_RTCVIDEOSOURCE_H_

#include <memory>

#include <absl/types/optional.h>
#include <nan.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/media/base/adaptedvideotracksource.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/peerconnectionfactory.h"  // IWYU pragma: keep

namespace webrtc { class VideoFrame; }

namespace node_webrtc {

struct RTCVideoSourceInit;

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
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
  const bool _is_screencast;
  const absl::optional<bool> _needs_denoising;
};

class RTCVideoSource
  : public Nan::ObjectWrap {
 public:
  RTCVideoSource();

  explicit RTCVideoSource(const RTCVideoSourceInit init);

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

#endif  // SRC_RTCVIDEOSOURCE_H_
