/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/datachannel.h"

#include <utility>

#include <webrtc/api/data_channel_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/rtc_base/copy_on_write_buffer.h>
#include <v8.h>

#include "src/common.h"
#include "src/error.h"
#include "src/errorfactory.h"
#include "src/events.h"

Nan::Persistent<v8::Function>& node_webrtc::DataChannel::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

node_webrtc::DataChannelObserver::DataChannelObserver(std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel)
  : EventQueue()
  , _factory(std::move(factory))
  , _jingleDataChannel(std::move(jingleDataChannel)) {
  TRACE_CALL;
  _jingleDataChannel->RegisterObserver(this);
  TRACE_END;
}

void node_webrtc::DataChannelObserver::OnStateChange() {
  auto state = _jingleDataChannel->state();
  Enqueue(node_webrtc::Callback1<node_webrtc::DataChannel>::Create([state](node_webrtc::DataChannel & channel) {
    node_webrtc::DataChannel::HandleStateChange(channel, state);
  }));
}

void node_webrtc::DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  Enqueue(node_webrtc::Callback1<node_webrtc::DataChannel>::Create([buffer](node_webrtc::DataChannel & channel) {
    node_webrtc::DataChannel::HandleMessage(channel, buffer);
  }));
}

static void requeue(node_webrtc::DataChannelObserver& observer, node_webrtc::DataChannel& channel) {
  while (auto event = observer.Dequeue()) {
    channel.Dispatch(std::move(event));
  }
}

node_webrtc::DataChannel::DataChannel(node_webrtc::DataChannelObserver* observer)
  : node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>("RTCDataChannel", *this)
  , _binaryType(BinaryType::kArrayBuffer)
  , _factory(observer->_factory)
  , _jingleDataChannel(observer->_jingleDataChannel) {
  _jingleDataChannel->RegisterObserver(this);

  // Re-queue cached observer events
  requeue(*observer, *this);

  delete observer;
}

node_webrtc::DataChannel::~DataChannel() {
  wrap()->Release(this);
}

void node_webrtc::DataChannel::CleanupInternals() {
  if (_jingleDataChannel == nullptr) {
    return;
  }
  _jingleDataChannel->UnregisterObserver();
  _cached_id = _jingleDataChannel->id();
  _cached_label = _jingleDataChannel->label();
  _cached_max_packet_life_time = _jingleDataChannel->maxRetransmitTime();
  _cached_max_retransmits = _jingleDataChannel->maxRetransmits();
  _cached_negotiated = _jingleDataChannel->negotiated();
  _cached_ordered = _jingleDataChannel->ordered();
  _cached_protocol = _jingleDataChannel->protocol();
  _cached_buffered_amount = _jingleDataChannel->buffered_amount();
  _jingleDataChannel = nullptr;
}

void node_webrtc::DataChannel::OnPeerConnectionClosed() {
  if (_jingleDataChannel != nullptr) {
    CleanupInternals();
    Stop();
  }
}

NAN_METHOD(node_webrtc::DataChannel::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the DataChannel.");
  }

  v8::Local<v8::External> _observer = v8::Local<v8::External>::Cast(info[0]);
  auto observer = static_cast<node_webrtc::DataChannelObserver*>(_observer->Value());

  auto obj = new node_webrtc::DataChannel(observer);
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

void node_webrtc::DataChannel::OnStateChange() {
  auto state = _jingleDataChannel->state();
  if (state == webrtc::DataChannelInterface::kClosed) {
    CleanupInternals();
  }
  Dispatch(node_webrtc::CreateCallback<node_webrtc::DataChannel>([this, state]() {
    node_webrtc::DataChannel::HandleStateChange(*this, state);
  }));
}

void node_webrtc::DataChannel::HandleStateChange(node_webrtc::DataChannel& channel, webrtc::DataChannelInterface::DataState state) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> argv[1];
  if (state == webrtc::DataChannelInterface::kClosed) {
    argv[0] = Nan::New("closed").ToLocalChecked();
    channel.MakeCallback("onstatechange", 1, argv);
  } else if (state == webrtc::DataChannelInterface::kOpen) {
    argv[0] = Nan::New("open").ToLocalChecked();
    channel.MakeCallback("onstatechange", 1, argv);
  }
  if (state == webrtc::DataChannelInterface::kClosed) {
    channel.Stop();
  }
}

void node_webrtc::DataChannel::OnMessage(const webrtc::DataBuffer& buffer) {
  Dispatch(node_webrtc::CreateCallback<node_webrtc::DataChannel>([this, buffer]() {
    node_webrtc::DataChannel::HandleMessage(*this, buffer);
  }));
}

void node_webrtc::DataChannel::HandleMessage(node_webrtc::DataChannel& channel, const webrtc::DataBuffer& buffer) {
  bool binary = buffer.binary;
  size_t size = buffer.size();
  std::unique_ptr<char[]> message = std::unique_ptr<char[]>(new char[size]);
  memcpy(reinterpret_cast<void*>(message.get()), reinterpret_cast<const void*>(buffer.data.data()), size);

  Nan::HandleScope scope;
  v8::Local<v8::Value> argv[1];
  if (binary) {
    v8::Local<v8::ArrayBuffer> array = v8::ArrayBuffer::New(
            v8::Isolate::GetCurrent(),
            message.release(),
            size,
            v8::ArrayBufferCreationMode::kInternalized);
    argv[0] = array;
  } else {
    v8::Local<v8::String> str = Nan::New(message.get(), size).ToLocalChecked();
    argv[0] = str;
  }
  channel.MakeCallback("onmessage", 1, argv);
}

NAN_METHOD(node_webrtc::DataChannel::Send) {
  TRACE_CALL;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.This());

  if (self->_jingleDataChannel != nullptr) {
    if (self->_jingleDataChannel->state() != webrtc::DataChannelInterface::DataState::kOpen) {
      TRACE_END;
      return Nan::ThrowError(node_webrtc::ErrorFactory::CreateInvalidStateError("RTCDataChannel.readyState is not 'open'"));
    }
    if (info[0]->IsString()) {
      v8::Local<v8::String> str = v8::Local<v8::String>::Cast(info[0]);
      std::string data = *v8::String::Utf8Value(str);

      webrtc::DataBuffer buffer(data);
      self->_jingleDataChannel->Send(buffer);
    } else {
      v8::Local<v8::ArrayBuffer> arraybuffer;
      size_t byte_offset = 0;
      size_t byte_length = 0;

      if (info[0]->IsArrayBufferView()) {
        v8::Local<v8::ArrayBufferView> view = v8::Local<v8::ArrayBufferView>::Cast(info[0]);
        arraybuffer = view->Buffer();
        byte_offset = view->ByteOffset();
        byte_length = view->ByteLength();
      } else if (info[0]->IsArrayBuffer()) {
        arraybuffer = v8::Local<v8::ArrayBuffer>::Cast(info[0]);
        byte_length = arraybuffer->ByteLength();
      } else {
        TRACE_END;
        return Nan::ThrowTypeError("Expected a Blob or ArrayBuffer");
      }

      v8::ArrayBuffer::Contents content = arraybuffer->GetContents();
      rtc::CopyOnWriteBuffer buffer(
          static_cast<char*>(content.Data()) + byte_offset,
          byte_length);

      webrtc::DataBuffer data_buffer(buffer, true);
      self->_jingleDataChannel->Send(data_buffer);
    }
  } else {
    TRACE_END;
    return Nan::ThrowError(node_webrtc::ErrorFactory::CreateInvalidStateError("RTCDataChannel.readyState is not 'open'"));
  }

  TRACE_END;
}

NAN_METHOD(node_webrtc::DataChannel::Close) {
  TRACE_CALL;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.This());

  if (self->_jingleDataChannel != nullptr) {
    self->_jingleDataChannel->Close();
  }

  TRACE_END;
}

NAN_GETTER(node_webrtc::DataChannel::GetBufferedAmount) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  uint64_t buffered_amount = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->buffered_amount()
      : self->_cached_buffered_amount;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<v8::Number>(buffered_amount));
}

NAN_GETTER(node_webrtc::DataChannel::GetId) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  auto id = self->_jingleDataChannel
      ? self->_jingleDataChannel->id()
      : self->_cached_id;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<v8::Number>(id));
}

NAN_GETTER(node_webrtc::DataChannel::GetLabel) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  std::string label = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->label()
      : self->_cached_label;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(label).ToLocalChecked());
}

NAN_GETTER(node_webrtc::DataChannel::GetMaxPacketLifeTime) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  auto max_packet_life_time = self->_jingleDataChannel
      ? self->_jingleDataChannel->maxRetransmitTime()
      : self->_cached_max_packet_life_time;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(max_packet_life_time));
}

NAN_GETTER(node_webrtc::DataChannel::GetMaxRetransmits) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  auto max_retransmits = self->_jingleDataChannel
      ? self->_jingleDataChannel->maxRetransmits()
      : self->_cached_max_retransmits;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(max_retransmits));
}

NAN_GETTER(node_webrtc::DataChannel::GetNegotiated) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  auto negotiated = self->_jingleDataChannel
      ? self->_jingleDataChannel->negotiated()
      : self->_cached_negotiated;

  TRACE_END;
  info.GetReturnValue().Set(negotiated);
}

NAN_GETTER(node_webrtc::DataChannel::GetOrdered) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  auto ordered = self->_jingleDataChannel
      ? self->_jingleDataChannel->ordered()
      : self->_cached_ordered;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(ordered));
}

NAN_GETTER(node_webrtc::DataChannel::GetPriority) {
  TRACE_CALL;
  (void) property;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New("high").ToLocalChecked());
}

NAN_GETTER(node_webrtc::DataChannel::GetProtocol) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  auto protocol = self->_jingleDataChannel
      ? self->_jingleDataChannel->protocol()
      : self->_cached_protocol;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(protocol).ToLocalChecked());
}

NAN_GETTER(node_webrtc::DataChannel::GetReadyState) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(self->_jingleDataChannel
      ? self->_jingleDataChannel->state()
      : webrtc::DataChannelInterface::kClosed,
      state,
      v8::Local<v8::Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
}

NAN_GETTER(node_webrtc::DataChannel::GetBinaryType) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(self->_binaryType, binaryType, v8::Local<v8::Value>);

  TRACE_END;
  info.GetReturnValue().Set(binaryType);
}

NAN_SETTER(node_webrtc::DataChannel::SetBinaryType) {
  TRACE_CALL;
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, binaryType, BinaryType);

  self->_binaryType = binaryType;

  TRACE_END;
}

NAN_SETTER(node_webrtc::DataChannel::ReadOnly) {
  (void) info;
  (void) property;
  (void) value;
  INFO("PeerConnection::ReadOnly");
}

node_webrtc::Wrap <
node_webrtc::DataChannel*,
rtc::scoped_refptr<webrtc::DataChannelInterface>,
node_webrtc::DataChannelObserver*
> * node_webrtc::DataChannel::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  node_webrtc::DataChannel*,
  rtc::scoped_refptr<webrtc::DataChannelInterface>,
  node_webrtc::DataChannelObserver*
  > (node_webrtc::DataChannel::Create);
  return wrap;
}

node_webrtc::DataChannel* node_webrtc::DataChannel::Create(
    node_webrtc::DataChannelObserver* observer,
    rtc::scoped_refptr<webrtc::DataChannelInterface>) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv = Nan::New<v8::External>(static_cast<void*>(observer));
  auto channel = Nan::NewInstance(Nan::New(node_webrtc::DataChannel::constructor()), 1, &cargv).ToLocalChecked();
  return node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::DataChannel>::Unwrap(channel);
}

void node_webrtc::DataChannel::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DataChannel").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "send", Send);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("bufferedAmount").ToLocalChecked(), GetBufferedAmount, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("label").ToLocalChecked(), GetLabel, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("maxPacketLifeTime").ToLocalChecked(), GetMaxPacketLifeTime, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("maxRetransmits").ToLocalChecked(), GetMaxRetransmits, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("negotiated").ToLocalChecked(), GetNegotiated, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ordered").ToLocalChecked(), GetOrdered, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("priority").ToLocalChecked(), GetPriority, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("protocol").ToLocalChecked(), GetProtocol, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("binaryType").ToLocalChecked(), GetBinaryType, SetBinaryType);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState, ReadOnly);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("DataChannel").ToLocalChecked(), tpl->GetFunction());
}
