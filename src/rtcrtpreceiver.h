/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCRTPRECEIVER_H_
#define SRC_RTCRTPRECEIVER_H_

#include <memory>

#include <nan.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>

#include "src/asyncobjectwrap.h"  // IWYU pragma: keep
#include "src/wrap.h"

namespace webrtc {

class RtpReceiverInterface;

}  // namespace webrtc

namespace node_webrtc {

class PeerConnectionFactory;

class RTCRtpReceiver: public node_webrtc::AsyncObjectWrap {
 public:
  ~RTCRtpReceiver() override;

  static void Init(v8::Handle<v8::Object> exports);

  // NOTE(mroberts): Working around an MSVC bug.
  static RTCRtpReceiver* Unwrap(v8::Local<v8::Object> object) {
    return node_webrtc::AsyncObjectWrap::Unwrap<RTCRtpReceiver>(object);
  }

  static ::node_webrtc::Wrap <
  RTCRtpReceiver*,
  rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > * wrap();

 private:
  RTCRtpReceiver(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::RtpReceiverInterface>&& receiver);

  static RTCRtpReceiver* Create(
      std::shared_ptr<PeerConnectionFactory>,
      rtc::scoped_refptr<webrtc::RtpReceiverInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetTrack);
  static NAN_GETTER(GetTransport);
  static NAN_GETTER(GetRtcpTransport);

  static NAN_METHOD(GetCapabilities);

  static NAN_METHOD(GetParameters);
  static NAN_METHOD(GetContributingSources);
  static NAN_METHOD(GetSynchronizationSources);
  static NAN_METHOD(GetStats);

  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::RtpReceiverInterface> _receiver;
};

}  // namespace node_webrtc

#endif  // SRC_RTCRTPRECEIVER_H_
