/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <webrtc/api/rtp_sender_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/converters/napi.h"
#include "src/node/async_object_wrap.h"
#include "src/node/wrap.h"

namespace webrtc { class RtpSenderInterface; }

namespace node_webrtc {

class PeerConnectionFactory;

class RTCRtpSender: public AsyncObjectWrap<RTCRtpSender> {
 public:
  explicit RTCRtpSender(const Napi::CallbackInfo&);

  ~RTCRtpSender() override;

  static void Init(Napi::Env, Napi::Object);

  rtc::scoped_refptr<webrtc::RtpSenderInterface> sender() { return _sender; }

  static ::node_webrtc::Wrap <
  RTCRtpSender*,
  rtc::scoped_refptr<webrtc::RtpSenderInterface>,
  PeerConnectionFactory*
  > * wrap();

  static Napi::FunctionReference& constructor();

 private:
  static RTCRtpSender* Create(
      PeerConnectionFactory*,
      rtc::scoped_refptr<webrtc::RtpSenderInterface>);

  Napi::Value GetTrack(const Napi::CallbackInfo&);
  Napi::Value GetTransport(const Napi::CallbackInfo&);
  Napi::Value GetRtcpTransport(const Napi::CallbackInfo&);

  static Napi::Value GetCapabilities(const Napi::CallbackInfo&);

  Napi::Value GetParameters(const Napi::CallbackInfo&);
  Napi::Value SetParameters(const Napi::CallbackInfo&);
  Napi::Value GetStats(const Napi::CallbackInfo&);
  Napi::Value ReplaceTrack(const Napi::CallbackInfo&);
  Napi::Value SetStreams(const Napi::CallbackInfo&);

  PeerConnectionFactory* _factory;
  rtc::scoped_refptr<webrtc::RtpSenderInterface> _sender;
};

DECLARE_TO_AND_FROM_NAPI(RTCRtpSender*)

}  // namespace node_webrtc
