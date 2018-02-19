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
#if NODE_MODULE_VERSION < 0x000C
Nan::Persistent<Function> DataChannel::ArrayBufferConstructor;
#endif

DataChannelObserver::DataChannelObserver(std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel): EventQueue() {
  TRACE_CALL;
  _factory = factory;
  _jingleDataChannel = jingleDataChannel;
  _jingleDataChannel->RegisterObserver(this);
  TRACE_END;
}

DataChannelObserver::~DataChannelObserver() {
  _jingleDataChannel = nullptr;
  _factory = nullptr;
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
  : EventLoop(*this),
    _binaryType(DataChannel::ARRAY_BUFFER) {
  _factory = observer->_factory;
  _jingleDataChannel = observer->_jingleDataChannel;
  _jingleDataChannel->RegisterObserver(this);

  // Re-queue cached observer events
  requeue(*observer, *this);

  delete observer;
}

DataChannel::~DataChannel() {
  TRACE_CALL;
  _jingleDataChannel->UnregisterObserver();
  _jingleDataChannel = nullptr;
  _factory = nullptr;
  TRACE_END;
}

void DataChannel::DidStop() {
  Unref();
}

NAN_METHOD(DataChannel::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the DataChannel.");
  }

  Local<External> _observer = Local<External>::Cast(info[0]);
  node_webrtc::DataChannelObserver* observer = static_cast<node_webrtc::DataChannelObserver*>(_observer->Value());

  DataChannel* obj = new DataChannel(observer);
  obj->Wrap(info.This());
  obj->Ref();

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

void DataChannel::HandleErrorEvent(const ErrorEvent<DataChannel>& event) const {
  TRACE_CALL;

  Nan::HandleScope scope;

  auto self = handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onerror").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
    Nan::Error(event.msg.c_str())
  };

  TRACE_END;
  Nan::MakeCallback(self, callback, 1, argv);
}

void DataChannel::HandleStateEvent(const DataChannelStateChangeEvent& event) {
  TRACE_CALL;

  Nan::HandleScope scope;

  if (event.state == webrtc::DataChannelInterface::kClosed) {
    Stop();
  }

  auto self = handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onstatechange").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
    Nan::New(event.state)
  };

  TRACE_END;
  Nan::MakeCallback(self, callback, 1, argv);
}

void DataChannel::HandleMessageEvent(const MessageEvent& event) const {
  TRACE_CALL;

  Nan::HandleScope scope;

  auto self = handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onmessage").ToLocalChecked()));

  Local<Value> argv[1];

  if (event.binary) {
#if NODE_MODULE_VERSION > 0x000B
    auto array = v8::ArrayBuffer::New(
            v8::Isolate::GetCurrent(), event.message.get(), event.size);
#else
    Local<Object> array = Nan::New(ArrayBufferConstructor)->NewInstance();
    array->SetIndexedPropertiesToExternalArrayData(
        event.message.get(), v8::kExternalByteArray, event.size);
    array->ForceSet(Nan::New("byteLength").ToLocalChecked(), Nan::New<Integer>(static_cast<uint32_t>(event.size)));
#endif

    argv[0] = array;
    Nan::MakeCallback(self, callback, 1, argv);
  } else {
    auto str = Nan::New(event.message.get(), static_cast<int>(event.size)).ToLocalChecked();

    argv[0] = str;
    Nan::MakeCallback(self, callback, 1, argv);
  }

  TRACE_END;
}

void DataChannel::OnStateChange() {
  TRACE_CALL;
  Dispatch(DataChannelStateChangeEvent::Create(_jingleDataChannel->state()));
  TRACE_END;
}

void DataChannel::OnMessage(const webrtc::DataBuffer& buffer) {
  TRACE_CALL;
  Dispatch(MessageEvent::Create(&buffer));
  TRACE_END;
}

NAN_METHOD(DataChannel::Send) {
  TRACE_CALL;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>(info.This());

  if (self->_jingleDataChannel != nullptr) {
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
        // TODO(mroberts): Throw a TypeError.
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

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>(info.This());

  if (self->_jingleDataChannel != nullptr) {
    self->_jingleDataChannel->Close();
  }

  TRACE_END;
  return;
}

NAN_GETTER(DataChannel::GetBufferedAmount) {
  TRACE_CALL;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  uint64_t buffered_amount = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->buffered_amount()
      : 0;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(buffered_amount));
}

NAN_GETTER(DataChannel::GetLabel) {
  TRACE_CALL;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  std::string label = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->label()
      : "";

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(label).ToLocalChecked());
}

NAN_GETTER(DataChannel::GetReadyState) {
  TRACE_CALL;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  webrtc::DataChannelInterface::DataState state = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->state()
      : webrtc::DataChannelInterface::kClosed;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(static_cast<uint32_t>(state)));
}

NAN_GETTER(DataChannel::GetBinaryType) {
  TRACE_CALL;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(static_cast<uint32_t>(self->_binaryType)));
}

NAN_SETTER(DataChannel::SetBinaryType) {
  TRACE_CALL;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());
  self->_binaryType = static_cast<BinaryType>(value->Uint32Value());

  TRACE_END;
}

NAN_SETTER(DataChannel::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void DataChannel::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("DataChannel").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "close", Close);
  Nan::SetPrototypeMethod(tpl, "send", Send);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("bufferedAmount").ToLocalChecked(), GetBufferedAmount, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("label").ToLocalChecked(), GetLabel, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("binaryType").ToLocalChecked(), GetBinaryType, SetBinaryType);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("DataChannel").ToLocalChecked(), tpl->GetFunction());

#if NODE_MODULE_VERSION < 0x000C
  Local<Object> global = Nan::GetCurrentContext()->Global();
  Local<Value> obj = global->Get(Nan::New("ArrayBuffer").ToLocalChecked());
  ArrayBufferConstructor.Reset(obj.As<Function>());
#endif
}
