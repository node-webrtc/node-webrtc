/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_DATACHANNEL_H_
#define SRC_DATACHANNEL_H_

#include <string.h>

#include <string>
#include <queue>

#include "nan.h"
#include "uv.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/datachannelinterface.h"
#include "webrtc/base/buffer.h"
#include "webrtc/base/scoped_ref_ptr.h"

#include "converters/webrtc.h"
#include "peerconnectionfactory.h"

namespace node_webrtc {

class DataChannelObserver;

class DataChannel
  : public Nan::ObjectWrap
  , public webrtc::DataChannelObserver {
  friend class node_webrtc::DataChannelObserver;
 public:
  struct ErrorEvent {
    explicit ErrorEvent(const std::string& msg)
      : msg(msg) {}

    std::string msg;
  };

  struct MessageEvent {
    explicit MessageEvent(const webrtc::DataBuffer* buffer) {
      binary = buffer->binary;
      size = buffer->size();
      message = std::shared_ptr<char>(new char[size], std::default_delete<char>());
      memcpy(static_cast<void*>(message.get()), static_cast<const void*>(buffer->data.data()), size);
    }

    bool binary;
    std::shared_ptr<char> message;
    size_t size;
  };

  struct StateEvent {
    explicit StateEvent(const webrtc::DataChannelInterface::DataState state)
      : state(state) {}

    webrtc::DataChannelInterface::DataState state;
  };

  enum AsyncEventType {
    MESSAGE = 0x1 << 0,  // 1
    ERROR = 0x1 << 1,  // 2
    STATE = 0x1 << 2,  // 4
  };

  explicit DataChannel(node_webrtc::DataChannelObserver* observer);
  ~DataChannel();

  //
  // DataChannelObserver implementation.
  //

  virtual void OnStateChange();
  virtual void OnMessage(const webrtc::DataBuffer& buffer);

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(Send);
  static NAN_METHOD(Close);

  static NAN_GETTER(GetBufferedAmount);
  static NAN_GETTER(GetId);
  static NAN_GETTER(GetLabel);
  static NAN_GETTER(GetMaxRetransmits);
  static NAN_GETTER(GetOrdered);
  static NAN_GETTER(GetPriority);
  static NAN_GETTER(GetProtocol);
  static NAN_GETTER(GetBinaryType);
  static NAN_GETTER(GetReadyState);
  static NAN_SETTER(SetBinaryType);
  static NAN_SETTER(ReadOnly);

  void QueueEvent(DataChannel::AsyncEventType type, void* data);

 private:
  static void Run(uv_async_t* handle, int status);
  static void Shutdown(uv_async_t* handle);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  uv_loop_t* loop;
  std::queue<AsyncEvent> _events;

  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  int _cached_id;
  std::string _cached_label;
  uint16_t _cached_max_retransmits;
  bool _cached_ordered;
  std::string _cached_protocol;
  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
  node_webrtc::BinaryType _binaryType;

#if NODE_MODULE_VERSION < 0x000C
  static Nan::Persistent<v8::Function> ArrayBufferConstructor;

#endif
};

class DataChannelObserver
  : public webrtc::DataChannelObserver {
  friend class node_webrtc::DataChannel;
 public:
  explicit DataChannelObserver(std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
      rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel);
  virtual ~DataChannelObserver();

  virtual void OnStateChange();
  virtual void OnMessage(const webrtc::DataBuffer& buffer);

 private:
  void QueueEvent(DataChannel::AsyncEventType type, void* data);

  uv_mutex_t lock;
  std::queue<DataChannel::AsyncEvent> _events;
  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
};

}  // namespace node_webrtc

#endif  // SRC_DATACHANNEL_H_
