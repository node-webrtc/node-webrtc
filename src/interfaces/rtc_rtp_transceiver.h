/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <node-addon-api/napi.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/converters/napi.h"
#include "src/converters/napi.h"
#include "src/node/async_object_wrap.h"
#include "src/node/wrap.h"

namespace webrtc { class RtpTransceiverInterface; }

namespace node_webrtc {

class PeerConnectionFactory;

class RTCRtpTransceiver: public AsyncObjectWrap<RTCRtpTransceiver> {
 public:
  explicit RTCRtpTransceiver(const Napi::CallbackInfo&);

  ~RTCRtpTransceiver() override;

  static void Init(Napi::Env, Napi::Object);

  static ::node_webrtc::Wrap <
  RTCRtpTransceiver*,
  rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
  PeerConnectionFactory*
  > * wrap();

  static Napi::FunctionReference& constructor();

 private:

  static RTCRtpTransceiver* Create(
      PeerConnectionFactory*,
      rtc::scoped_refptr<webrtc::RtpTransceiverInterface>);

  Napi::Value GetMid(const Napi::CallbackInfo&);
  Napi::Value GetSender(const Napi::CallbackInfo&);
  Napi::Value GetReceiver(const Napi::CallbackInfo&);
  Napi::Value GetStopped(const Napi::CallbackInfo&);
  Napi::Value GetDirection(const Napi::CallbackInfo&);
  void SetDirection(const Napi::CallbackInfo&, const Napi::Value&);
  Napi::Value GetCurrentDirection(const Napi::CallbackInfo&);

  Napi::Value Stop(const Napi::CallbackInfo&);
  Napi::Value SetCodecPreferences(const Napi::CallbackInfo&);

  PeerConnectionFactory* _factory;
  rtc::scoped_refptr<webrtc::RtpTransceiverInterface> _transceiver;
};

DECLARE_TO_NAPI(RTCRtpTransceiver*)

}  // namespace node_webrtc
