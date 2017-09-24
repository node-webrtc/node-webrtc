/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_DATACHANNEL_H_
#define SRC_DATACHANNEL_H_

#include <nan.h>
#include <uv.h>
#include <v8.h>

#include <cstring>
#include <string>
#include <queue>

#include "webrtc/api/datachannelinterface.h"
#include "webrtc/base/buffer.h"
#include "webrtc/base/scoped_ref_ptr.h"

#include "src/eventloop.h"
#include "src/events.h"

namespace node_webrtc {

class DataChannelObserver;

class DataChannel
: public EventLoop<DataChannel>
, public Nan::ObjectWrap
, public webrtc::DataChannelObserver {
  friend class node_webrtc::DataChannelObserver;

 public:
  enum BinaryType {
    BLOB = 0x0,
    ARRAY_BUFFER = 0x1
  };

  explicit DataChannel(node_webrtc::DataChannelObserver* observer);
  ~DataChannel() override;

  //
  // DataChannelObserver implementation.
  //

  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static void Dispose();
  static Nan::Persistent<v8::Function>* constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(Send);
  static NAN_METHOD(Close);
  static NAN_METHOD(Shutdown);

  static NAN_GETTER(GetBufferedAmount);
  static NAN_GETTER(GetLabel);
  static NAN_GETTER(GetBinaryType);
  static NAN_GETTER(GetReadyState);
  static NAN_SETTER(SetBinaryType);
  static NAN_SETTER(ReadOnly);

  void HandleErrorEvent(const ErrorEvent<DataChannel>& event) const;
  void HandleStateEvent(const DataChannelStateChangeEvent& event);
  void HandleMessageEvent(const MessageEvent& event) const;

 protected:
  void DidStop() override;

 private:
  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
  BinaryType _binaryType;

#if NODE_MODULE_VERSION < 0x000C
  static Nan::Persistent<v8::Function> ArrayBufferConstructor;

#endif
};

class DataChannelObserver
: public EventQueue<DataChannel>
, public webrtc::DataChannelObserver {
 public:
  explicit DataChannelObserver(rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel);
  ~DataChannelObserver() override;

  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;

  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
};

}  // namespace node_webrtc

#endif  // SRC_DATACHANNEL_H_
