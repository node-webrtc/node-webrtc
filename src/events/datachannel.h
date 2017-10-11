/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_EVENTS_DATACHANNEL_H_
#define SRC_EVENTS_DATACHANNEL_H_

#include <nan.h>

#include <memory>

#include "webrtc/api/peerconnectioninterface.h"

#include "src/events.h"

namespace node_webrtc {

class DataChannel;

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

#endif  // SRC_EVENTS_DATACHANNEL_H_
