/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCRTPTRANSCEIVER_H_
#define SRC_RTCRTPTRANSCEIVER_H_

#include <memory>

#include <nan.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>

#include "src/asyncobjectwrap.h"  // IWYU pragma: keep

namespace webrtc {

class RtpTransceiverInterface;

}  // namespace webrtc

namespace node_webrtc {

class MediaStreamTrack;
class PeerConnectionFactory;

class RTCRtpTransceiver: public node_webrtc::AsyncObjectWrap {
 public:
  RTCRtpTransceiver(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::RtpTransceiverInterface>&& transceiver);

  ~RTCRtpTransceiver() override;

  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_GETTER(GetMid);
  static NAN_GETTER(GetSender);
  static NAN_GETTER(GetReceiver);
  static NAN_GETTER(GetStopped);
  static NAN_GETTER(GetDirection);
  static NAN_GETTER(GetCurrentDirection);

  static NAN_METHOD(Stop);
  static NAN_METHOD(SetCodecPreferences);

  // NOTE(mroberts): Working around an MSVC bug.
  static RTCRtpTransceiver* Unwrap(v8::Local<v8::Object> object) {
    return node_webrtc::AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(object);
  }

 private:
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::RtpTransceiverInterface> _transceiver;
};

}  // namespace node_webrtc

#endif  // SRC_RTCRTPTRANSCEIVER_H_
