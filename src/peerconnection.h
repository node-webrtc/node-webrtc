/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_PEERCONNECTION_H_
#define SRC_PEERCONNECTION_H_

#include <cstdint>

#include <string>
#include <queue>

#include <nan.h>
#include <v8.h>

#include "webrtc/api/datachannelinterface.h"
#include "webrtc/api/jsep.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/statstypes.h"
#include "webrtc/base/scoped_ref_ptr.h"

#include "src/events/peerconnection.h"
#include "src/promisefulfillingeventloop.h"

namespace node_webrtc {

class CreateOfferObserver;
class CreateAnswerObserver;
class DataChannelObserver;
class SetLocalDescriptionObserver;
class SetRemoteDescriptionObserver;

class PeerConnection
: public PromiseFulfillingEventLoop<PeerConnection>
, public Nan::ObjectWrap
, public webrtc::PeerConnectionObserver {
 public:

  explicit PeerConnection(webrtc::PeerConnectionInterface::RTCConfiguration configuration);
  ~PeerConnection() override;

  //
  // PeerConnectionObserver implementation.
  //

  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
  void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnRenegotiationNeeded() override;

  void OnDataChannel(webrtc::DataChannelInterface* data_channel) override;

  void OnAddStream(webrtc::MediaStreamInterface* stream) override;
  void OnRemoveStream(webrtc::MediaStreamInterface* stream) override;

  //
  // Nodejs wrapping.
  //
  static void Init(rtc::Thread* signalingThread, rtc::Thread* workerThread, v8::Handle<v8::Object> exports);
  static void Dispose();
  static Nan::Persistent<v8::Function>* constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(CreateOffer);
  static NAN_METHOD(CreateAnswer);
  static NAN_METHOD(SetLocalDescription);
  static NAN_METHOD(SetRemoteDescription);
  static NAN_METHOD(UpdateIce);
  static NAN_METHOD(AddIceCandidate);
  static NAN_METHOD(CreateDataChannel);
  /*
  static NAN_METHOD(GetLocalStreams);
  static NAN_METHOD(GetRemoteStreams);
  static NAN_METHOD(GetStreamById);
  static NAN_METHOD(AddStream);
  static NAN_METHOD(RemoveStream);
  */
  static NAN_METHOD(GetStats);
  static NAN_METHOD(Close);

  static NAN_GETTER(GetLocalDescription);
  static NAN_GETTER(GetRemoteDescription);
  static NAN_GETTER(GetIceConnectionState);
  static NAN_GETTER(GetSignalingState);
  static NAN_GETTER(GetIceGatheringState);
  static NAN_SETTER(ReadOnly);

  void HandleGetStatsEvent(const GetStatsEvent& event) const;
  void HandleSignalingStateChangeEvent(const SignalingStateChangeEvent& event);
  void HandleIceConnectionStateChangeEvent(const IceConnectionStateChangeEvent& event) const;
  void HandleIceGatheringStateChangeEvent(const IceGatheringStateChangeEvent& event) const;
  void HandleIceCandidateEvent(const IceEvent& event) const;
  void HandleDataChannelEvent(const DataChannelEvent& event) const;

 private:
  webrtc::AudioDeviceModule *_audioDeviceModule;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> _jinglePeerConnection;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _jinglePeerConnectionFactory;

  static rtc::Thread* _signalingThread;
  static rtc::Thread* _workerThread;
};

}  // namespace node_webrtc

#endif  // SRC_PEERCONNECTION_H_
