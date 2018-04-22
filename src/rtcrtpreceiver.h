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
#include "peerconnectionfactory.h"
#include "promisefulfillingeventloop.h"
#include "mediastreamtrack.h"

namespace node_webrtc {

class RTCRtpReceiver: public Nan::ObjectWrap {
 public:
  RTCRtpReceiver(
      std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
      rtc::scoped_refptr<webrtc::RtpReceiverInterface>&& receiver,
      node_webrtc::MediaStreamTrack* track);

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

 private:
  bool _closed;
  const std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  const rtc::scoped_refptr<webrtc::RtpReceiverInterface> _receiver;
  const std::shared_ptr<node_webrtc::MediaStreamTrack> _track;
};

}  // namespace node_webrtc

#endif  // SRC_RTCRTPRECEIVER_H_
