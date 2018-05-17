/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCRTPRECEIVER_H_
#define SRC_RTCRTPRECEIVER_H_

#include "nan.h"
#include "v8.h"

#include "src/asyncobjectwrap.h"
#include "src/mediastreamtrack.h"
#include "src/peerconnectionfactory.h"
#include "src/promisefulfillingeventloop.h"

namespace node_webrtc {

class RTCRtpReceiver: public node_webrtc::AsyncObjectWrap {
 public:
  RTCRtpReceiver(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::RtpReceiverInterface>&& receiver,
      node_webrtc::MediaStreamTrack* track);

  ~RTCRtpReceiver() override;

  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_GETTER(GetTrack);
  static NAN_GETTER(GetTransport);
  static NAN_GETTER(GetRtcpTransport);

  static NAN_METHOD(GetCapabilities);

  static NAN_METHOD(GetParameters);
  static NAN_METHOD(GetContributingSources);
  static NAN_METHOD(GetSynchronizationSources);
  static NAN_METHOD(GetStats);

  /**
   * This method is called when the RTCPeerConnection that created the underlying RTCRtpReceiver is closed. Calling this
   * method sets _closed, which protects against subsequent accesses to getSources in GetContributingSources and
   * GetSynchronizationSources.
   */
  void OnPeerConnectionClosed();

  // NOTE(mroberts): Working around an MSVC bug.
  static RTCRtpReceiver* Unwrap(v8::Local<v8::Object> object) {
    return node_webrtc::AsyncObjectWrap::Unwrap<RTCRtpReceiver>(object);
  }

 private:
  bool _closed;
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::RtpReceiverInterface> _receiver;
  node_webrtc::MediaStreamTrack* _track;
};

}  // namespace node_webrtc

#endif  // SRC_RTCRTPRECEIVER_H_
