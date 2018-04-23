/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
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

#include "converters.h"
#include "converters/v8.h"
#include "error.h"
#include "functional/either.h"
#include "functional/validation.h"
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
  virtual void Dispatch(T&) {
    // Do nothing.
  }

  virtual ~Event() = default;

  static std::unique_ptr<Event<T>> Create() {
    return std::unique_ptr<Event<T>>(new Event<T>());
  }
};

/**
 * A PromiseEvent can be dispatched to a PromiseFulfillingEventLoop in order to
 * resolve or reject a Promise.
 * @tparam T the PromiseFulfillingEventLoop type
 * @tparam L the type of values representing failure
 * @tparam R the type of values representing success
 */
template <typename T, typename R = node_webrtc::Undefined, typename L = node_webrtc::SomeError>
class PromiseEvent: public Event<T> {
 public:
  void Dispatch(T&) override {
    Nan::HandleScope scope;
    if (_resolver) {
      auto resolver = Nan::New(*_resolver);
      _result.template FromEither<void>([resolver](L error) {
        CONVERT_OR_REJECT_AND_RETURN(resolver, error, value, v8::Local<v8::Value>);
        resolver->Reject(value);
      }, [resolver](R result) {
        CONVERT_OR_REJECT_AND_RETURN(resolver, result, value, v8::Local<v8::Value>);
        resolver->Resolve(value);
      });
    }
  }

  void Reject(L error) {
    _result = node_webrtc::Either<L, R>::Left(error);
  }

  void Resolve(R result) {
    _result = node_webrtc::Either<L, R>::Right(result);
  }

  static std::pair<v8::Local<v8::Promise::Resolver>, std::unique_ptr<PromiseEvent<T, R, L>>> Create() {
    Nan::EscapableHandleScope scope;
    auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()->GetIsolate());
    auto event = std::unique_ptr<PromiseEvent<T, R, L>>(new PromiseEvent<T, R, L>(
                std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>>(
                    new Nan::Persistent<v8::Promise::Resolver>(resolver))));
    return std::pair<v8::Local<v8::Promise::Resolver>, std::unique_ptr<PromiseEvent<T, R, L>>>(
            scope.Escape(resolver),
            std::move(event));
  }

 protected:
  explicit PromiseEvent(std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> resolver)
    : _resolver(std::move(resolver)) {}

 private:
  std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
  node_webrtc::Either<L, R> _result;
};

class NegotiationNeededEvent: public Event<PeerConnection> {
 public:
  void Dispatch(PeerConnection&) override;

  static std::unique_ptr<NegotiationNeededEvent> Create() {
    return std::unique_ptr<NegotiationNeededEvent>(new NegotiationNeededEvent());
  }

 private:
  NegotiationNeededEvent() = default;
};

template <typename T>
class ErrorEvent: public Event<T> {
 public:
  const std::string msg;

  void Dispatch(T&) override {}

 protected:
  explicit ErrorEvent(const std::string&& msg): msg(msg) {}
};

class MessageEvent: public Event<DataChannel> {
 public:
  bool binary;
  std::unique_ptr<char[]> message;
  size_t size;

  void Dispatch(DataChannel& dataChannel) override;

  static std::unique_ptr<MessageEvent> Create(const webrtc::DataBuffer* buffer) {
    return std::unique_ptr<MessageEvent>(new MessageEvent(buffer));
  }

 private:
  explicit MessageEvent(const webrtc::DataBuffer* buffer) {
    binary = buffer->binary;
    size = buffer->size();
    message = std::unique_ptr<char[]>(new char[size]);
    memcpy(reinterpret_cast<void*>(message.get()), reinterpret_cast<const void*>(buffer->data.data()), size);
  }
};

class IceEvent: public Event<PeerConnection> {
 public:
  std::string error;
  std::unique_ptr<const webrtc::IceCandidateInterface> candidate;

  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<IceEvent> Create(const webrtc::IceCandidateInterface* ice_candidate) {
    return std::unique_ptr<IceEvent>(new IceEvent(ice_candidate));
  }

 private:
  explicit IceEvent(const webrtc::IceCandidateInterface* ice_candidate) {
    std::string sdp;
    if (!ice_candidate->ToString(&sdp)) {
      error = "Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc";
      return;
    }
    webrtc::SdpParseError parseError;
    candidate = std::unique_ptr<const webrtc::IceCandidateInterface>(
            webrtc::CreateIceCandidate(ice_candidate->sdp_mid(), ice_candidate->sdp_mline_index(), sdp, &parseError));
    if (!parseError.description.empty()) {
      error = parseError.description;
    } else if (!candidate) {
      error = "Failed to copy RTCIceCandidate";
    }
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

class OnAddTrackEvent: public Event<PeerConnection> {
 public:
  rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver;
  const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>> streams;

  void Dispatch(PeerConnection& peerConnection) override;

  static std::unique_ptr<OnAddTrackEvent> Create(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
    return std::unique_ptr<OnAddTrackEvent>(new OnAddTrackEvent(receiver, streams));
  }

 private:
  explicit OnAddTrackEvent(
      rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
      const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams)
    : receiver(receiver)
    , streams(streams) {}
};

}  // namespace node_webrtc

#endif  // SRC_EVENTS_H_