/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_DATACHANNEL_H_
#define SRC_DATACHANNEL_H_

#include <iosfwd>
#include <memory>

#include <nan.h>
#include <webrtc/api/datachannelinterface.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>

#include "src/asyncobjectwrapwithloop.h"  // IWYU pragma: keep
#include "src/converters/webrtc.h"
#include "src/eventqueue.h"  // IWYU pragma: keep
#include "src/wrap.h"

namespace node_webrtc {

class DataChannelObserver;
class DataChannelStateChangeEvent;
class MessageEvent;
class PeerConnectionFactory;

template <typename T> class ErrorEvent;

class DataChannel
  : public node_webrtc::AsyncObjectWrapWithLoop<DataChannel>
  , public webrtc::DataChannelObserver {
  friend class node_webrtc::DataChannelObserver;
 public:
  DataChannel() = delete;

  DataChannel(DataChannel const&) = delete;

  DataChannel& operator=(DataChannel const&) = delete;

  ~DataChannel() override;

  static void Init(v8::Handle<v8::Object> exports);

  //
  // DataChannelObserver implementation.
  //
  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;

  void HandleErrorEvent(const ErrorEvent<DataChannel>& event);
  void HandleStateEvent(const DataChannelStateChangeEvent& event);
  void HandleMessageEvent(MessageEvent& event);

  void OnPeerConnectionClosed();

  static ::node_webrtc::Wrap <
  DataChannel*,
  rtc::scoped_refptr<webrtc::DataChannelInterface>,
  node_webrtc::DataChannelObserver*
  > * wrap();

 private:
  explicit DataChannel(node_webrtc::DataChannelObserver* observer);

  static DataChannel* Create(
      node_webrtc::DataChannelObserver*,
      rtc::scoped_refptr<webrtc::DataChannelInterface>);

  static Nan::Persistent<v8::Function>& constructor();

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

  node_webrtc::BinaryType _binaryType;
  int _cached_id;
  std::string _cached_label;
  uint16_t _cached_max_retransmits;
  bool _cached_ordered;
  std::string _cached_protocol;
  uint64_t _cached_buffered_amount;
  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
};

class DataChannelObserver
  : public node_webrtc::EventQueue<DataChannel>
  , public webrtc::DataChannelObserver {
  friend class node_webrtc::DataChannel;
 public:
  explicit DataChannelObserver(std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
      rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel);

  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;

  rtc::scoped_refptr<webrtc::DataChannelInterface> channel() { return _jingleDataChannel; }

 private:
  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
};

}  // namespace node_webrtc

#endif  // SRC_DATACHANNEL_H_
