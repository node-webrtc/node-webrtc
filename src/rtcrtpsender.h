/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCRTPSENDER_H_
#define SRC_RTCRTPSENDER_H_

#include <memory>

#include <nan.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/asyncobjectwrap.h"
#include "src/wrap.h"

namespace webrtc {

class RtpSenderInterface;

}  // namespace webrtc

namespace node_webrtc {

class PeerConnectionFactory;

class RTCRtpSender: public node_webrtc::AsyncObjectWrap {
 public:
  ~RTCRtpSender() override;

  static void Init(v8::Handle<v8::Object> exports);

  rtc::scoped_refptr<webrtc::RtpSenderInterface> sender() { return _sender; }

  static ::node_webrtc::Wrap <
  RTCRtpSender*,
  rtc::scoped_refptr<webrtc::RtpSenderInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

 private:
  RTCRtpSender(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::RtpSenderInterface>&& sender);

  static RTCRtpSender* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::RtpSenderInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetTrack);
  static NAN_GETTER(GetTransport);
  static NAN_GETTER(GetRtcpTransport);

  static NAN_METHOD(GetCapabilities);

  static NAN_METHOD(GetParameters);
  static NAN_METHOD(SetParameters);
  static NAN_METHOD(GetStats);
  static NAN_METHOD(ReplaceTrack);

  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::RtpSenderInterface> _sender;
};

}  // namespace node_webrtc

#endif  // SRC_RTCRTPSENDER_H_
