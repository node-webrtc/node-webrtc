/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCAUDIOSOURCE_H_
#define SRC_RTCAUDIOSOURCE_H_

#include <memory>

#include <nan.h>
#include <webrtc/api/mediastreaminterface.h>
#include <webrtc/pc/localaudiosource.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/peerconnectionfactory.h"  // IWYU pragma: keep

namespace node_webrtc {

struct RTCAudioSourceInit;

class RTCAudioTrackSource : public webrtc::LocalAudioSource {
 public:
  RTCAudioTrackSource() = default;

  ~RTCAudioTrackSource() override = default;

  SourceState state() const override {
    return webrtc::MediaSourceInterface::SourceState::kLive;
  }

  bool remote() const override {
    return false;
  }

  void PushData() {}

 private:
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
};

class RTCAudioSource
  : public Nan::ObjectWrap {
 public:
  RTCAudioSource();

  explicit RTCAudioSource(const RTCAudioSourceInit init);

  ~RTCAudioSource() override = default;

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);

 private:
  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_METHOD(CreateTrack);
  static NAN_METHOD(OnData);

  rtc::scoped_refptr<RTCAudioTrackSource> _source;
};

}  // namespace node_webrtc

#endif  // SRC_RTCAUDIOSOURCE_H_
