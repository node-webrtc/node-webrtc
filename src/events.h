/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTS_H_
#define SRC_EVENTS_H_

#include <iosfwd>

#include <nan.h>
#include <webrtc/api/datachannelinterface.h>
#include <webrtc/api/jsep.h>
#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/video/video_frame.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>

#include "src/converters/dictionaries.h"
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/functional/either.h"

namespace webrtc {

class RtpReceiverInterface;
class RtpTransceiverInterface;

}  // namespace webrtc

namespace node_webrtc {

class DataChannel;
class RTCAudioSink;
class RTCVideoSink;

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

template <typename T>
class Callback: public Event<T> {
 public:
  void Dispatch(T&) override {
    _callback();
  }

  static std::unique_ptr<Callback<T>> Create(std::function<void()> callback) {
    return std::unique_ptr<Callback<T>>(new Callback(std::move(callback)));
  }

 private:
  explicit Callback(std::function<void()> callback): _callback(std::move(callback)) {}
  const std::function<void()> _callback;
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
        resolver->Reject(Nan::GetCurrentContext(), value).IsNothing();
      }, [resolver](R result) {
        CONVERT_OR_REJECT_AND_RETURN(resolver, result, value, v8::Local<v8::Value>);
        resolver->Resolve(Nan::GetCurrentContext(), value).IsNothing();
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
    auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
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

class OnFrameEvent: public Event<RTCVideoSink> {
 public:
  const webrtc::VideoFrame frame;

  void Dispatch(RTCVideoSink& sink) override;

  static std::unique_ptr<OnFrameEvent> Create(const webrtc::VideoFrame& frame) {
    return std::unique_ptr<OnFrameEvent>(new OnFrameEvent(frame));
  }

 private:
  explicit OnFrameEvent(const webrtc::VideoFrame& frame): frame(frame) {}
};

class OnDataEvent: public Event<RTCAudioSink> {
 public:
  RTCOnDataEventDict dict;

  void Dispatch(RTCAudioSink& sink) override;

  static std::unique_ptr<OnDataEvent> Create(const RTCOnDataEventDict& dict) {
    return std::unique_ptr<OnDataEvent>(new OnDataEvent(dict));
  }

 private:
  OnDataEvent(const RTCOnDataEventDict& dict): dict(dict) {}
};

}  // namespace node_webrtc

#endif  // SRC_EVENTS_H_
