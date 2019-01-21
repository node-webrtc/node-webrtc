/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_PEERCONNECTION_H_
#define SRC_PEERCONNECTION_H_

#include <memory>

#include <nan.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/asyncobjectwrapwithloop.h"  // IWYU pragma: keep
#include "src/converters/dictionaries.h"

namespace webrtc {

class DataChannelInterface;
class IceCandidateInterface;
class MediaStreamInterface;
class RtpReceiverInterface;
class RtpTransceiverInterface;

}  // namespace webrtc

namespace node_webrtc {

class IceConnectionStateChangeEvent;
class IceEvent;
class IceGatheringStateChangeEvent;
class DataChannelEvent;
class NegotiationNeededEvent;
class OnAddTrackEvent;
class PeerConnectionFactory;
class SignalingStateChangeEvent;

class PeerConnection
  : public node_webrtc::AsyncObjectWrapWithLoop<PeerConnection>
  , public webrtc::PeerConnectionObserver {
 public:
  ~PeerConnection() override;

  //
  // PeerConnectionObserver implementation.
  //
  void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
  void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
  void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
  void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
  void OnRenegotiationNeeded() override;

  void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel) override;

  void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;
  void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override;

  void OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) override;
  void OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);

  void HandleIceConnectionStateChangeEvent(const IceConnectionStateChangeEvent& event);
  void HandleIceGatheringStateChangeEvent(const IceGatheringStateChangeEvent& event);
  void HandleIceCandidateEvent(const IceEvent& event);
  void HandleDataChannelEvent(const DataChannelEvent& event);
  void HandleNegotiationNeededEvent(const NegotiationNeededEvent& event);
  void HandleOnAddTrackEvent(const OnAddTrackEvent& event);
  void HandleSignalingStateChangeEvent(const SignalingStateChangeEvent& event);

  void SaveLastSdp(const RTCSessionDescriptionInit& lastSdp);

 private:
  explicit PeerConnection(ExtendedRTCConfiguration configuration);

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_METHOD(AddTrack);
  static NAN_METHOD(AddTransceiver);
  static NAN_METHOD(RemoveTrack);
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
  static NAN_METHOD(GetConfiguration);
  static NAN_METHOD(SetConfiguration);
  static NAN_METHOD(GetReceivers);
  static NAN_METHOD(GetSenders);
  static NAN_METHOD(GetStats);
  static NAN_METHOD(LegacyGetStats);
  static NAN_METHOD(GetTransceivers);
  static NAN_METHOD(Close);

  static NAN_GETTER(GetCanTrickleIceCandidates);
  static NAN_GETTER(GetConnectionState);
  static NAN_GETTER(GetCurrentLocalDescription);
  static NAN_GETTER(GetLocalDescription);
  static NAN_GETTER(GetPendingLocalDescription);
  static NAN_GETTER(GetCurrentRemoteDescription);
  static NAN_GETTER(GetRemoteDescription);
  static NAN_GETTER(GetPendingRemoteDescription);
  static NAN_GETTER(GetIceConnectionState);
  static NAN_GETTER(GetSignalingState);
  static NAN_GETTER(GetIceGatheringState);
  static NAN_SETTER(ReadOnly);

  RTCSessionDescriptionInit _lastSdp;

  UnsignedShortRange _port_range;
  ExtendedRTCConfiguration _cached_configuration;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> _jinglePeerConnection;

  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  bool _shouldReleaseFactory;

  std::vector<node_webrtc::DataChannel*> _channels;
};

}  // namespace node_webrtc

#endif  // SRC_PEERCONNECTION_H_
