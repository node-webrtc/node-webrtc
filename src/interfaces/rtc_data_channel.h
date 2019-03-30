/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <iosfwd>
#include <memory>

#include <nan.h>
#include <webrtc/api/data_channel_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <v8.h>

#include "src/enums/node_webrtc/binary_type.h"
#include "src/node/async_object_wrap_with_loop.h"
#include "src/node/event_queue.h"
#include "src/node/wrap.h"

namespace node_webrtc {

class DataChannelObserver;
class PeerConnectionFactory;

class RTCDataChannel
  : public AsyncObjectWrapWithLoop<RTCDataChannel>
  , public webrtc::DataChannelObserver {
  friend class node_webrtc::DataChannelObserver;
 public:
  RTCDataChannel() = delete;

  RTCDataChannel(RTCDataChannel const&) = delete;

  RTCDataChannel& operator=(RTCDataChannel const&) = delete;

  ~RTCDataChannel() override;

  static void Init(v8::Handle<v8::Object> exports);

  //
  // DataChannelObserver implementation.
  //
  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;

  void OnPeerConnectionClosed();

  static ::node_webrtc::Wrap <
  RTCDataChannel*,
  rtc::scoped_refptr<webrtc::DataChannelInterface>,
  node_webrtc::DataChannelObserver*
  > * wrap();

 private:
  explicit RTCDataChannel(node_webrtc::DataChannelObserver* observer);

  static RTCDataChannel* Create(
      node_webrtc::DataChannelObserver*,
      rtc::scoped_refptr<webrtc::DataChannelInterface>);

  static Nan::Persistent<v8::Function>& constructor();

  static void HandleStateChange(RTCDataChannel&, webrtc::DataChannelInterface::DataState);
  static void HandleMessage(RTCDataChannel&, const webrtc::DataBuffer& buffer);

  static NAN_METHOD(New);

  static NAN_METHOD(Send);
  static NAN_METHOD(Close);

  static NAN_GETTER(GetBufferedAmount);
  static NAN_GETTER(GetId);
  static NAN_GETTER(GetLabel);
  static NAN_GETTER(GetMaxPacketLifeTime);
  static NAN_GETTER(GetMaxRetransmits);
  static NAN_GETTER(GetNegotiated);
  static NAN_GETTER(GetOrdered);
  static NAN_GETTER(GetPriority);
  static NAN_GETTER(GetProtocol);
  static NAN_GETTER(GetBinaryType);
  static NAN_GETTER(GetReadyState);
  static NAN_SETTER(SetBinaryType);

  void CleanupInternals();

  BinaryType _binaryType;
  int _cached_id;
  std::string _cached_label;
  uint16_t _cached_max_packet_life_time;
  uint16_t _cached_max_retransmits;
  bool _cached_negotiated;
  bool _cached_ordered;
  std::string _cached_protocol;
  uint64_t _cached_buffered_amount;
  std::shared_ptr<PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
};

class DataChannelObserver
  : public EventQueue<RTCDataChannel>
  , public webrtc::DataChannelObserver {
  friend class RTCDataChannel;
 public:
  explicit DataChannelObserver(std::shared_ptr<PeerConnectionFactory> factory,
      rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel);

  void OnStateChange() override;
  void OnMessage(const webrtc::DataBuffer& buffer) override;

  rtc::scoped_refptr<webrtc::DataChannelInterface> channel() { return _jingleDataChannel; }

 private:
  std::shared_ptr<PeerConnectionFactory> _factory;
  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
};

}  // namespace node_webrtc
