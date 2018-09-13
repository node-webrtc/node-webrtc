/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "datachannel.h"

#include <stdint.h>

#include "common.h"
#include "converters.h"
#include "converters/webrtc.h"
#include "error.h"

using node_webrtc::AsyncObjectWrapWithLoop;
using node_webrtc::DataChannel;
using node_webrtc::DataChannelObserver;
using node_webrtc::DataChannelStateChangeEvent;
using node_webrtc::ErrorEvent;
using node_webrtc::Event;
using node_webrtc::MessageEvent;
using node_webrtc::StateEvent;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Integer;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;

Nan::Persistent<Function> DataChannel::constructor;

DataChannelObserver::DataChannelObserver(std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel)
  : EventQueue()
  , _factory(factory)
  , _jingleDataChannel(jingleDataChannel) {
  TRACE_CALL;
  _jingleDataChannel->RegisterObserver(this);
  TRACE_END;
}

void DataChannelObserver::OnStateChange() {
  TRACE_CALL;
  Enqueue(DataChannelStateChangeEvent::Create(_jingleDataChannel->state()));
  TRACE_END;
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  TRACE_CALL;
  Enqueue(MessageEvent::Create(&buffer));
  TRACE_END;
}

void requeue(DataChannelObserver& observer, DataChannel& channel) {
  while (auto event = observer.Dequeue()) {
    channel.Dispatch(std::move(event));
  }
}

DataChannel::DataChannel(node_webrtc::DataChannelObserver* observer)
  : AsyncObjectWrapWithLoop("RTCDataChannel", *this)
  , _binaryType(BinaryType::kArrayBuffer)
  , _factory(observer->_factory)
  , _jingleDataChannel(observer->_jingleDataChannel) {
  _jingleDataChannel->RegisterObserver(this);

  // Re-queue cached observer events
  requeue(*observer, *this);

  delete observer;
}

NAN_METHOD(DataChannel::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the DataChannel.");
  }

  Local<External> _observer = Local<External>::Cast(info[0]);
  auto observer = static_cast<node_webrtc::DataChannelObserver*>(_observer->Value());

  auto obj = new DataChannel(observer);
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

void DataChannel::HandleErrorEvent(const ErrorEvent<DataChannel>& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  Local<Value> argv[1];
  argv[0] = Nan::Error(Nan::New(event.msg).ToLocalChecked());
  MakeCallback("onerror", 1, argv);
  TRACE_END;
}

void DataChannel::HandleStateEvent(const DataChannelStateChangeEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  Local<Value> argv[1];
  if (event.state == webrtc::DataChannelInterface::kClosed) {
    argv[0] = Nan::New("closed").ToLocalChecked();
    MakeCallback("onstatechange", 1, argv);
  } else if (event.state == webrtc::DataChannelInterface::kOpen) {
    argv[0] = Nan::New("open").ToLocalChecked();
    MakeCallback("onstatechange", 1, argv);
  }

  if (event.state == webrtc::DataChannelInterface::kClosed) {
    Stop();
  }
  TRACE_END;
}

void DataChannel::HandleMessageEvent(MessageEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  Local<Value> argv[1];
  if (event.binary) {
    Local<v8::ArrayBuffer> array = v8::ArrayBuffer::New(
            v8::Isolate::GetCurrent(),
            event.message.release(),
            event.size,
            v8::ArrayBufferCreationMode::kInternalized);
    argv[0] = array;
  } else {
    Local<String> str = Nan::New(event.message.get(), event.size).ToLocalChecked();
    argv[0] = str;
  }
  MakeCallback("onmessage", 1, argv);
  TRACE_END;
}

void DataChannel::OnStateChange() {
  TRACE_CALL;
  auto state = _jingleDataChannel->state();
  Dispatch(DataChannelStateChangeEvent::Create(state));
  if (state == webrtc::DataChannelInterface::kClosed) {
    _jingleDataChannel->UnregisterObserver();
    _cached_id = _jingleDataChannel->id();
    _cached_label = _jingleDataChannel->label();
    _cached_max_retransmits = _jingleDataChannel->maxRetransmits();
    _cached_ordered = _jingleDataChannel->ordered();
    _cached_protocol = _jingleDataChannel->protocol();
    _jingleDataChannel = nullptr;
  }
  TRACE_END;
}

void DataChannel::OnMessage(const webrtc::DataBuffer& buffer) {
  TRACE_CALL;
  Dispatch(MessageEvent::Create(&buffer));
  TRACE_END;
}

NAN_METHOD(DataChannel::Send) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.This());

  if (self->_jingleDataChannel != nullptr) {
    if (self->_jingleDataChannel->state() != webrtc::DataChannelInterface::DataState::kOpen) {
      TRACE_END;
      return Nan::ThrowError("InvalidStateError");
    }
    if (info[0]->IsString()) {
      Local<String> str = Local<String>::Cast(info[0]);
      std::string data = *String::Utf8Value(str);

      webrtc::DataBuffer buffer(data);
      self->_jingleDataChannel->Send(buffer);
    } else {
      Local<v8::ArrayBuffer> arraybuffer;
      size_t byte_offset = 0;
      size_t byte_length = 0;

      if (info[0]->IsArrayBufferView()) {
        Local<v8::ArrayBufferView> view = Local<v8::ArrayBufferView>::Cast(info[0]);
        arraybuffer = view->Buffer();
        byte_offset = view->ByteOffset();
        byte_length = view->ByteLength();
      } else if (info[0]->IsArrayBuffer()) {
        arraybuffer = Local<v8::ArrayBuffer>::Cast(info[0]);
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
  }

  TRACE_END;
}

NAN_METHOD(DataChannel::Close) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.This());

  if (self->_jingleDataChannel != nullptr) {
    self->_jingleDataChannel->Close();
  }

  TRACE_END;
}

NAN_GETTER(DataChannel::GetBufferedAmount) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  uint64_t buffered_amount = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->buffered_amount()
      : 0;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(buffered_amount));
}

NAN_GETTER(DataChannel::GetId) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  auto id = self->_jingleDataChannel
      ? self->_jingleDataChannel->id()
      : self->_cached_id;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(id));
}

NAN_GETTER(DataChannel::GetLabel) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  std::string label = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->label()
      : self->_cached_label;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(label).ToLocalChecked());
}

NAN_GETTER(DataChannel::GetMaxRetransmits) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  auto max_retransmits = self->_jingleDataChannel
      ? self->_jingleDataChannel->maxRetransmits()
      : self->_cached_max_retransmits;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(max_retransmits));
}

NAN_GETTER(DataChannel::GetOrdered) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  auto ordered = self->_jingleDataChannel
      ? self->_jingleDataChannel->ordered()
      : self->_cached_ordered;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(ordered));
}

NAN_GETTER(DataChannel::GetPriority) {
  TRACE_CALL;
  (void) property;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New("high").ToLocalChecked());
}

NAN_GETTER(DataChannel::GetProtocol) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  auto protocol = self->_jingleDataChannel
      ? self->_jingleDataChannel->protocol()
      : self->_cached_protocol;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(protocol).ToLocalChecked());
}

NAN_GETTER(DataChannel::GetReadyState) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(self->_jingleDataChannel
      ? self->_jingleDataChannel->state()
      : webrtc::DataChannelInterface::kClosed,
      state,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
}

NAN_GETTER(DataChannel::GetBinaryType) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(self->_binaryType, binaryType, Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(binaryType);
}

NAN_SETTER(DataChannel::SetBinaryType) {
  TRACE_CALL;
  (void) property;

  auto self = AsyncObjectWrapWithLoop<DataChannel>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, binaryType, BinaryType);

  self->_binaryType = binaryType;

  TRACE_END;
}

NAN_SETTER(DataChannel::ReadOnly) {
  (void) info;
  (void) property;
  (void) value;
  INFO("PeerConnection::ReadOnly");
}

void DataChannel::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DataChannel").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "send", Send);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("bufferedAmount").ToLocalChecked(), GetBufferedAmount, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("label").ToLocalChecked(), GetLabel, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("maxRetransmits").ToLocalChecked(), GetMaxRetransmits, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("ordered").ToLocalChecked(), GetOrdered, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("priority").ToLocalChecked(), GetPriority, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("protocol").ToLocalChecked(), GetProtocol, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("binaryType").ToLocalChecked(), GetBinaryType, SetBinaryType);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("DataChannel").ToLocalChecked(), tpl->GetFunction());
}
