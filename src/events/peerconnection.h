/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTS_PEERCONNECTION_H_
#define SRC_EVENTS_PEERCONNECTION_H_

#include <memory>

#include "nan.h"
#include "v8.h"
#include "webrtc/api/peerconnectioninterface.h"

#include "src/events.h"
#include "src/events/promise.h"

namespace node_webrtc {

class DataChannelObserver;
class PeerConnection;

class SetLocalDescriptionSuccessEvent: public PromiseEvent<PeerConnection> {
 public:
  static std::unique_ptr<SetLocalDescriptionSuccessEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver) {
    return std::unique_ptr<SetLocalDescriptionSuccessEvent>(new SetLocalDescriptionSuccessEvent(std::move(resolver)));

  }

 private:
  explicit SetLocalDescriptionSuccessEvent(std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver)
      : PromiseEvent<PeerConnection>(std::move(resolver)) {}
};

class SetRemoteDescriptionSuccessEvent: public PromiseEvent<PeerConnection> {
 public:
  static std::unique_ptr<SetRemoteDescriptionSuccessEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver) {
    return std::unique_ptr<SetRemoteDescriptionSuccessEvent>(new SetRemoteDescriptionSuccessEvent(std::move(resolver)));
  }

 private:
  explicit SetRemoteDescriptionSuccessEvent(std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver)
      : PromiseEvent<PeerConnection>(std::move(resolver)) {}
};

class CreateAnswerErrorEvent: public PromiseRejectionEvent<PeerConnection> {
 public:
  static std::unique_ptr<CreateAnswerErrorEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      const std::string& msg) {
    return std::unique_ptr<CreateAnswerErrorEvent>(new CreateAnswerErrorEvent(std::move(resolver), std::string(msg)));
  }

 private:
  explicit CreateAnswerErrorEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver
      , const std::string&& msg)
      : PromiseRejectionEvent(std::move(resolver), std::string(msg)) {}
};

class CreateOfferErrorEvent: public PromiseRejectionEvent<PeerConnection> {
 public:
  static std::unique_ptr<CreateOfferErrorEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      const std::string& msg) {
    return std::unique_ptr<CreateOfferErrorEvent>(new CreateOfferErrorEvent(std::move(resolver), std::string(msg)));
  }

 private:
  explicit CreateOfferErrorEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver
      , const std::string&& msg)
      : PromiseRejectionEvent(std::move(resolver), std::string(msg)) {}
};

class SetLocalDescriptionErrorEvent: public PromiseRejectionEvent<PeerConnection> {
 public:
  static std::unique_ptr<SetLocalDescriptionErrorEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      const std::string reason) {
    return std::unique_ptr<SetLocalDescriptionErrorEvent>(
        new SetLocalDescriptionErrorEvent(std::move(resolver), reason));
  }

 private:
  explicit SetLocalDescriptionErrorEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver
      , const std::string reason)
      : PromiseRejectionEvent<PeerConnection>(std::move(resolver), reason) {}
};

class SetRemoteDescriptionErrorEvent: public PromiseRejectionEvent<PeerConnection> {
 public:
  static std::unique_ptr<SetRemoteDescriptionErrorEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      const std::string reason) {
    return std::unique_ptr<SetRemoteDescriptionErrorEvent>(
        new SetRemoteDescriptionErrorEvent(std::move(resolver), reason));
  }

 private:
  explicit SetRemoteDescriptionErrorEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver
      , const std::string reason)
      : PromiseRejectionEvent<PeerConnection>(std::move(resolver), reason) {}
};

class SdpEvent: public PromiseEvent<PeerConnection> {
 public:
  void Dispatch(PeerConnection& peerConnection) override;

 protected:
  explicit SdpEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver
      , std::unique_ptr<webrtc::SessionDescriptionInterface> sdp)
      : PromiseEvent(std::move(resolver))
      , _sdp(std::move(sdp)) {}

 private:
  std::unique_ptr<webrtc::SessionDescriptionInterface> _sdp;
};

class CreateAnswerSuccessEvent: public SdpEvent {
 public:
  static std::unique_ptr<CreateAnswerSuccessEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      std::unique_ptr<webrtc::SessionDescriptionInterface> sdp) {
    return std::unique_ptr<CreateAnswerSuccessEvent>(new CreateAnswerSuccessEvent(std::move(resolver), std::move(sdp)));
  }

 private:
  explicit CreateAnswerSuccessEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      std::unique_ptr<webrtc::SessionDescriptionInterface> sdp)
      : SdpEvent(std::move(resolver), std::move(sdp)) {}
};

class CreateOfferSuccessEvent: public SdpEvent {
 public:
  static std::unique_ptr<CreateOfferSuccessEvent> Create(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      std::unique_ptr<webrtc::SessionDescriptionInterface> sdp) {
    return std::unique_ptr<CreateOfferSuccessEvent>(new CreateOfferSuccessEvent(std::move(resolver), std::move(sdp)));
  }

 private:
  explicit CreateOfferSuccessEvent(
      std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver,
      std::unique_ptr<webrtc::SessionDescriptionInterface> sdp)
      : SdpEvent(std::move(resolver), std::move(sdp)) {}
};

class IceEvent: public Event<PeerConnection> {
 public:
  const uint32_t sdpMLineIndex;
  const std::string sdpMid;
  std::string candidate = "";

  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<IceEvent> Create(const webrtc::IceCandidateInterface* ice_candidate) {
    return std::unique_ptr<IceEvent>(new IceEvent(ice_candidate));
  }

 private:
  explicit IceEvent(const webrtc::IceCandidateInterface* ice_candidate)
      : sdpMLineIndex(static_cast<uint32_t>(ice_candidate->sdp_mline_index()))
      , sdpMid(ice_candidate->sdp_mid()) {
    ice_candidate->ToString(&candidate);
  }
};

class IceConnectionStateChangeEvent
    : public StateEvent<PeerConnection, webrtc::PeerConnectionInterface::IceConnectionState> {
 public:
  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<IceConnectionStateChangeEvent> Create(
      const webrtc::PeerConnectionInterface::IceConnectionState state) {
    return std::unique_ptr<IceConnectionStateChangeEvent>(new IceConnectionStateChangeEvent(state));
  }

 private:
  explicit IceConnectionStateChangeEvent(const webrtc::PeerConnectionInterface::IceConnectionState state)
      : StateEvent(state) {}
};

class IceGatheringStateChangeEvent
    : public StateEvent<PeerConnection, webrtc::PeerConnectionInterface::IceGatheringState> {
 public:
  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<IceGatheringStateChangeEvent> Create(
      const webrtc::PeerConnectionInterface::IceGatheringState state) {
    return std::unique_ptr<IceGatheringStateChangeEvent>(new IceGatheringStateChangeEvent(state));
  }

 private:
  explicit IceGatheringStateChangeEvent(const webrtc::PeerConnectionInterface::IceGatheringState state)
      : StateEvent(state) {}
};

class SignalingStateChangeEvent: public StateEvent<PeerConnection, webrtc::PeerConnectionInterface::SignalingState> {
 public:
  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<SignalingStateChangeEvent> Create(
      const webrtc::PeerConnectionInterface::SignalingState state) {
    return std::unique_ptr<SignalingStateChangeEvent>(new SignalingStateChangeEvent(state));
  }

 private:
  explicit SignalingStateChangeEvent(const webrtc::PeerConnectionInterface::SignalingState state): StateEvent(state) {}
};

class DataChannelEvent: public Event<PeerConnection> {
 public:
  DataChannelObserver* observer;

  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<DataChannelEvent> Create(DataChannelObserver* observer) {
    return std::unique_ptr<DataChannelEvent>(new DataChannelEvent(observer));
  }

 private:
  explicit DataChannelEvent(DataChannelObserver* observer): observer(observer) {}
};

class GetStatsEvent: public Event<PeerConnection> {
 public:
  const Nan::Callback* callback;
  const webrtc::StatsReports reports;

  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<GetStatsEvent> Create(const Nan::Callback* callback, const webrtc::StatsReports& reports) {
    return std::unique_ptr<GetStatsEvent>(new GetStatsEvent(callback, reports));
  }

 private:
  GetStatsEvent(const Nan::Callback* callback, const webrtc::StatsReports& reports)
      : callback(callback), reports(reports) {}
};

}  // namespace node_webrtc

#endif  // SRC_EVENTS_PEERCONNECTION_H_
