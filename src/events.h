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

#include "webrtc/api/peerconnectioninterface.h"

namespace node_webrtc {

class DataChannel;
class DataChannelObserver;

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

}  // namespace node_webrtc

#endif  // SRC_EVENTS_H_