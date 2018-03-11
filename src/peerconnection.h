/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_PEERCONNECTION_H_
#define SRC_PEERCONNECTION_H_

#include <stdint.h>

#include <string>
#include <queue>

#include "nan.h"
#include "uv.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/datachannelinterface.h"  // IWYU pragma: keep
#include "webrtc/api/jsep.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/statstypes.h"
#include "webrtc/base/scoped_ref_ptr.h"

#include "eventloop.h"
#include "events.h"
#include "peerconnectionfactory.h"

namespace node_webrtc {

class CreateOfferObserver;
class CreateAnswerObserver;
class DataChannelObserver;
class SetLocalDescriptionObserver;
class SetRemoteDescriptionObserver;

class PeerConnection
  : public EventLoop<PeerConnection>
  , public Nan::ObjectWrap
  , public webrtc::PeerConnectionObserver {
 public:
  explicit PeerConnection(webrtc::PeerConnectionInterface::IceServers iceServerList);
  ~PeerConnection();

  //
  // PeerConnectionObserver implementation.
  //

  virtual void OnError();

  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
  void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnRenegotiationNeeded() override;

  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

  void OnAddStream(webrtc::MediaStreamInterface* stream) override;
  void OnRemoveStream(webrtc::MediaStreamInterface* stream) override;

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
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

  void HandleErrorEvent(const ErrorEvent<PeerConnection>& event) const;
  void HandleSdpEvent(const SdpEvent& event) const;
  void HandleGetStatsEvent(const GetStatsEvent& event) const;
  void HandleVoidEvent() const;
  void HandleSignalingStateChangeEvent(const SignalingStateChangeEvent& event);
  void HandleIceConnectionStateChangeEvent(const IceConnectionStateChangeEvent& event) const;
  void HandleIceGatheringStateChangeEvent(const IceGatheringStateChangeEvent& event) const;
  void HandleIceCandidateEvent(const IceEvent& event) const;
  void HandleDataChannelEvent(const DataChannelEvent& event) const;

 protected:
  void DidStop() override;

 private:
  webrtc::PeerConnectionInterface::IceServers _iceServers;

  rtc::scoped_refptr<CreateOfferObserver> _createOfferObserver;
  rtc::scoped_refptr<CreateAnswerObserver> _createAnswerObserver;
  rtc::scoped_refptr<SetLocalDescriptionObserver> _setLocalDescriptionObserver;
  rtc::scoped_refptr<SetRemoteDescriptionObserver> _setRemoteDescriptionObserver;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> _jinglePeerConnection;

  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  bool _shouldReleaseFactory;
};

}  // namespace node_webrtc

#endif  // SRC_PEERCONNECTION_H_
