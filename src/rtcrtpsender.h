/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCRTPSENDER_H_
#define SRC_RTCRTPSENDER_H_

#include "nan.h"
#include "v8.h"

#include "src/asyncobjectwrap.h"
#include "src/peerconnectionfactory.h"
#include "src/mediastreamtrack.h"

namespace node_webrtc {

class RTCRtpSender: public node_webrtc::AsyncObjectWrap {
 public:
  RTCRtpSender(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::RtpSenderInterface>&& sender,
      node_webrtc::MediaStreamTrack* track);

  ~RTCRtpSender() override;

  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_GETTER(GetTrack);
  static NAN_GETTER(GetTransport);
  static NAN_GETTER(GetRtcpTransport);

  static NAN_METHOD(GetCapabilities);

  static NAN_METHOD(GetParameters);
  static NAN_METHOD(SetParameters);
  static NAN_METHOD(GetStats);
  static NAN_METHOD(ReplaceTrack);

  rtc::scoped_refptr<webrtc::RtpSenderInterface> sender() { return _sender; }

 private:
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::RtpSenderInterface> _sender;
  node_webrtc::MediaStreamTrack* _track;
};

}  // namespace node_webrtc

#endif  // SRC_RTCRTPSENDER_H_
