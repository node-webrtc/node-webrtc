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
#include <node-addon-api/napi.h>
#include <webrtc/api/scoped_refptr.h>
#include <v8.h>

#include "src/converters/napi.h"
#include "src/converters/v8.h"
#include "src/node/napi_async_object_wrap.h"
#include "src/node/wrap.h"

namespace webrtc { class RtpReceiverInterface; }

namespace node_webrtc {

class PeerConnectionFactory;

class RTCRtpReceiver: public napi::AsyncObjectWrap<RTCRtpReceiver> {
 public:
  explicit RTCRtpReceiver(const Napi::CallbackInfo&);

  ~RTCRtpReceiver() override;

  static void Init(Napi::Env, Napi::Object);

  static ::node_webrtc::Wrap <
  RTCRtpReceiver*,
  rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

  static Napi::FunctionReference& constructor();

 private:
  static RTCRtpReceiver* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::RtpReceiverInterface>);

  Napi::Value GetTrack(const Napi::CallbackInfo&);
  Napi::Value GetTransport(const Napi::CallbackInfo&);
  Napi::Value GetRtcpTransport(const Napi::CallbackInfo&);

  static Napi::Value GetCapabilities(const Napi::CallbackInfo&);

  Napi::Value GetParameters(const Napi::CallbackInfo&);
  Napi::Value GetContributingSources(const Napi::CallbackInfo&);
  Napi::Value GetSynchronizationSources(const Napi::CallbackInfo&);
  Napi::Value GetStats(const Napi::CallbackInfo&);

  std::shared_ptr<PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::RtpReceiverInterface> _receiver;
};

DECLARE_TO_JS(RTCRtpReceiver*)
DECLARE_TO_AND_FROM_NAPI(RTCRtpReceiver*)

}  // namespace node_webrtc
