/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <nan.h>
#include <webrtc/api/scoped_refptr.h>
#include <v8.h>

#include "src/node/async_object_wrap.h"
#include "src/node/wrap.h"

namespace webrtc {

class RtpTransceiverInterface;

}  // namespace webrtc

namespace node_webrtc {

class PeerConnectionFactory;

class RTCRtpTransceiver: public AsyncObjectWrap {
 public:
  ~RTCRtpTransceiver() override;

  static void Init(v8::Handle<v8::Object> exports);

  // NOTE(mroberts): Working around an MSVC bug.
  static RTCRtpTransceiver* Unwrap(v8::Local<v8::Object> object) {
    return AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(object);
  }

  static ::node_webrtc::Wrap <
  RTCRtpTransceiver*,
  rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

  static Nan::Persistent<v8::FunctionTemplate>& tpl();

 private:
  RTCRtpTransceiver(
      std::shared_ptr<PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::RtpTransceiverInterface>&& transceiver);

  static RTCRtpTransceiver* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::RtpTransceiverInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetMid);
  static NAN_GETTER(GetSender);
  static NAN_GETTER(GetReceiver);
  static NAN_GETTER(GetStopped);
  static NAN_GETTER(GetDirection);
  static NAN_SETTER(SetDirection);
  static NAN_GETTER(GetCurrentDirection);

  static NAN_METHOD(Stop);
  static NAN_METHOD(SetCodecPreferences);

  const std::shared_ptr<PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::RtpTransceiverInterface> _transceiver;
};

}  // namespace node_webrtc
