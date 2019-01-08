/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCVIDEOSOURCE_H_
#define SRC_RTCVIDEOSOURCE_H_

#include <nan.h>
#include <v8.h>  // IWYU pragma: keep

#include <webrtc/media/base/adaptedvideotracksource.h>

#include "src/peerconnectionfactory.h"
#include "src/webrtc/fakeframesource.h"

namespace node_webrtc {

class RTCVideoTrackSource : public rtc::AdaptedVideoTrackSource {
 public:
  RTCVideoTrackSource(): rtc::AdaptedVideoTrackSource() {}

  ~RTCVideoTrackSource() override = default;

  SourceState state() const override {
    return webrtc::MediaSourceInterface::SourceState::kLive;
  }

  bool remote() const override {
    return false;
  }

  bool is_screencast() const override {
    return false;
  }

  absl::optional<bool> needs_denoising() const override {
    return absl::optional<bool>();
  }

  void TriggerOnFrame() {
    this->OnFrame(_frame_source.GetFrame());
  }

 private:
  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
  node_webrtc::FakeFrameSource _frame_source = node_webrtc::FakeFrameSource(1280, 720, cricket::VideoFormat::FpsToInterval(30) / rtc::kNumNanosecsPerMicrosec);
};

class RTCVideoSource
  : public Nan::ObjectWrap {
 public:
  RTCVideoSource();

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
  static NAN_GETTER(GetState);
  static NAN_GETTER(GetRemote);

  static NAN_METHOD(CreateTrack);
  static NAN_METHOD(OnFrame);

  rtc::scoped_refptr<RTCVideoTrackSource> _source;
};

}  // namespace node_webrtc

#endif  // SRC_RTCVIDEOSOURCE_H_
