/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTS_H_
#define SRC_EVENTS_H_

#include <nan.h>

#include <memory>

#include "webrtc/api/peerconnectioninterface.h"

namespace node_webrtc {

class DataChannel;
class DataChannelObserver;
class PeerConnection;

/**
 * Event represents an event that can be dispatched to a target.
 * @tparam T the target type
 */
template<typename T>
class Event {
 public:
  /**
   * Dispatch the Event to the target.
   * @param target the target to dispatch to
   */
  virtual void Dispatch(T& target) {
    // Do nothing.
  }
};

class AddIceCandidateSuccessEvent: public Event<PeerConnection> {
 public:
  static std::unique_ptr<AddIceCandidateSuccessEvent> Create() {
    return std::unique_ptr<AddIceCandidateSuccessEvent>(new AddIceCandidateSuccessEvent());
  }

 private:
  AddIceCandidateSuccessEvent() = default;
};

class SetLocalDescriptionSuccessEvent: public Event<PeerConnection> {
 public:
  static std::unique_ptr<SetLocalDescriptionSuccessEvent> Create() {
    return std::unique_ptr<SetLocalDescriptionSuccessEvent>(new SetLocalDescriptionSuccessEvent());
  }

 private:
  SetLocalDescriptionSuccessEvent() = default;
};

class SetRemoteDescriptionSuccessEvent: public Event<PeerConnection> {
 public:
  static std::unique_ptr<SetRemoteDescriptionSuccessEvent> Create() {
    return std::unique_ptr<SetRemoteDescriptionSuccessEvent>(new SetRemoteDescriptionSuccessEvent());
  }

 private:
  SetRemoteDescriptionSuccessEvent() = default;
};

template <typename T>
class ErrorEvent: public Event<T> {
 public:
  const std::string msg;

  void Dispatch(T&) override {}

 protected:
  explicit ErrorEvent(const std::string&& msg): msg(msg) {}
};

class AddIceCandidateErrorEvent: public ErrorEvent<PeerConnection> {
 public:
  static std::unique_ptr<AddIceCandidateErrorEvent> Create(const std::string& msg) {
    return std::unique_ptr<AddIceCandidateErrorEvent>(new AddIceCandidateErrorEvent(std::string(msg)));
  }

 private:
  explicit AddIceCandidateErrorEvent(const std::string&& msg): ErrorEvent(std::string(msg)) {}
};

class CreateAnswerErrorEvent: public ErrorEvent<PeerConnection> {
 public:
  static std::unique_ptr<CreateAnswerErrorEvent> Create(const std::string& msg) {
    return std::unique_ptr<CreateAnswerErrorEvent>(new CreateAnswerErrorEvent(std::string(msg)));
  }

 private:
  explicit CreateAnswerErrorEvent(const std::string&& msg): ErrorEvent(std::string(msg)) {}
};

class CreateOfferErrorEvent: public ErrorEvent<PeerConnection> {
 public:
  static std::unique_ptr<CreateOfferErrorEvent> Create(const std::string& msg) {
    return std::unique_ptr<CreateOfferErrorEvent>(new CreateOfferErrorEvent(std::string(msg)));
  }

 private:
  explicit CreateOfferErrorEvent(const std::string&& msg): ErrorEvent(std::string(msg)) {}
};

class SetLocalDescriptionErrorEvent: public ErrorEvent<PeerConnection> {
 public:
  static std::unique_ptr<SetLocalDescriptionErrorEvent> Create(const std::string& msg) {
    return std::unique_ptr<SetLocalDescriptionErrorEvent>(new SetLocalDescriptionErrorEvent(std::string(msg)));
  }

 private:
  explicit SetLocalDescriptionErrorEvent(const std::string&& msg): ErrorEvent(std::string(msg)) {}
};

class SetRemoteDescriptionErrorEvent: public ErrorEvent<PeerConnection> {
 public:
  static std::unique_ptr<SetRemoteDescriptionErrorEvent> Create(const std::string& msg) {
    return std::unique_ptr<SetRemoteDescriptionErrorEvent>(new SetRemoteDescriptionErrorEvent(std::string(msg)));
  }

 private:
  explicit SetRemoteDescriptionErrorEvent(const std::string&& msg): ErrorEvent(std::string(msg)) {}
};

class MessageEvent: public Event<DataChannel> {
 public:
  bool binary;
  char* message;
  size_t size;

  void Dispatch(DataChannel& dataChannel) override;

  static std::unique_ptr<MessageEvent> Create(const webrtc::DataBuffer* buffer) {
    return std::unique_ptr<MessageEvent>(new MessageEvent(buffer));
  }

 private:
  explicit MessageEvent(const webrtc::DataBuffer* buffer) {
    binary = buffer->binary;
    size = buffer->size();
    message = new char[size];
    memcpy(reinterpret_cast<void*>(message), reinterpret_cast<const void*>(buffer->data.data()), size);
  }
};

class SdpEvent: public Event<PeerConnection> {
 public:
  const std::string type;
  std::string desc = "";

  void Dispatch(PeerConnection& peerConnection) override;

 protected:
  explicit SdpEvent(webrtc::SessionDescriptionInterface* sdp)
      : type(sdp->type()) {
    sdp->ToString(&desc);
  }
};

class CreateAnswerSuccessEvent: public SdpEvent {
 public:
  static std::unique_ptr<CreateAnswerSuccessEvent> Create(webrtc::SessionDescriptionInterface* sdp) {
    return std::unique_ptr<CreateAnswerSuccessEvent>(new CreateAnswerSuccessEvent(sdp));
  }

 private:
  explicit CreateAnswerSuccessEvent(webrtc::SessionDescriptionInterface* sdp): SdpEvent(sdp) {}
};

class CreateOfferSuccessEvent: public SdpEvent {
 public:
  static std::unique_ptr<CreateOfferSuccessEvent> Create(webrtc::SessionDescriptionInterface* sdp) {
    return std::unique_ptr<CreateOfferSuccessEvent>(new CreateOfferSuccessEvent(sdp));
  }

 private:
  explicit CreateOfferSuccessEvent(webrtc::SessionDescriptionInterface* sdp): SdpEvent(sdp) {}
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

template <typename T, typename S>
class StateEvent: public Event<T> {
 public:
  const S state;

 protected:
  explicit StateEvent(const S state): state(state) {}
};

class DataChannelStateChangeEvent: public StateEvent<DataChannel, webrtc::DataChannelInterface::DataState> {
 public:
  void Dispatch(DataChannel& dataChannel) override;

  static std::unique_ptr<DataChannelStateChangeEvent> Create(const webrtc::DataChannelInterface::DataState state) {
    return std::unique_ptr<DataChannelStateChangeEvent>(new DataChannelStateChangeEvent(state));
  }

 protected:
  explicit DataChannelStateChangeEvent(const webrtc::DataChannelInterface::DataState state): StateEvent(state) {}
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

#endif  // SRC_EVENTS_H_
