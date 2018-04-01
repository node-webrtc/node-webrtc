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

using node_webrtc::DataChannel;
using node_webrtc::DataChannelObserver;
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
    rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel) {
  TRACE_CALL;
  uv_mutex_init(&lock);
  _factory = factory;
  _jingleDataChannel = jingleDataChannel;
  _jingleDataChannel->RegisterObserver(this);
  TRACE_END;
}

DataChannelObserver::~DataChannelObserver() {
  _factory = nullptr;
  _jingleDataChannel = nullptr;
  uv_mutex_destroy(&lock);
}

void DataChannelObserver::OnStateChange() {
  TRACE_CALL;
  auto data = new DataChannel::StateEvent(_jingleDataChannel->state());
  QueueEvent(DataChannel::STATE, static_cast<void*>(data));
  TRACE_END;
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  TRACE_CALL;
  auto data = new DataChannel::MessageEvent(&buffer);
  QueueEvent(DataChannel::MESSAGE, static_cast<void*>(data));
  TRACE_END;
}

void DataChannelObserver::QueueEvent(DataChannel::AsyncEventType type, void* data) {
  TRACE_CALL;
  DataChannel::AsyncEvent evt = { type, data };
  uv_mutex_lock(&lock);
  _events.push(evt);
  uv_mutex_unlock(&lock);
  TRACE_END;
}

DataChannel::DataChannel(node_webrtc::DataChannelObserver* observer)
  : loop(uv_default_loop()),
    _binaryType(BinaryType::kArrayBuffer) {
  uv_mutex_init(&lock);
  uv_async_init(loop, &async, reinterpret_cast<uv_async_cb>(Run));

  _factory = observer->_factory;
  _jingleDataChannel = observer->_jingleDataChannel;
  _jingleDataChannel->RegisterObserver(this);

  async.data = this;

  // Re-queue cached observer events
  while (true) {
    uv_mutex_lock(&observer->lock);
    if (observer->_events.empty()) {
      uv_mutex_unlock(&observer->lock);
      break;
    }
    auto evt = observer->_events.front();
    observer->_events.pop();
    uv_mutex_unlock(&observer->lock);
    QueueEvent(evt.type, evt.data);
  }

  delete observer;
}

DataChannel::~DataChannel() {
  TRACE_CALL;
  _factory = nullptr;
  _jingleDataChannel = nullptr;
  uv_mutex_destroy(&lock);
  TRACE_END;
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
  obj->Ref();

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

void DataChannel::QueueEvent(AsyncEventType type, void* data) {
  TRACE_CALL;
  AsyncEvent evt = { type, data };
  uv_mutex_lock(&lock);
  _events.push(evt);
  uv_mutex_unlock(&lock);

  uv_async_send(&async);
  TRACE_END;
}

/*NAN_WEAK_CALLBACK(MessageWeakCallback)
{
  P* parameter = data.GetParameter();
  delete[] parameter->message;
  parameter->message = nullptr;
  delete parameter;
  //Nan::AdjustExternalMemory(-parameter->size);
}*/

void DataChannel::Run(uv_async_t* handle, int status) {
  Nan::HandleScope scope;

  auto self = static_cast<DataChannel*>(handle->data);
  TRACE_CALL_P((uintptr_t)self);
  auto do_shutdown = false;

  while (true) {
    auto dc = self->handle();

    uv_mutex_lock(&self->lock);
    if (self->_events.empty()) {
      uv_mutex_unlock(&self->lock);
      break;
    }
    auto evt = self->_events.front();
    self->_events.pop();
    uv_mutex_unlock(&self->lock);

    TRACE_U("evt.type", evt.type);
    if (DataChannel::ERROR & evt.type) {
      auto data = static_cast<DataChannel::ErrorEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(dc->Get(Nan::New("onerror").ToLocalChecked()));
      Local<Value> argv[1];
      argv[0] = Nan::Error(data->msg.c_str());
      Nan::MakeCallback(dc, callback, 1, argv);
    } else if (DataChannel::STATE & evt.type) {
      auto data = static_cast<StateEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(dc->Get(Nan::New("onstatechange").ToLocalChecked()));
      Local<Value> argv[1];
      Local<Integer> state = Nan::New<Integer>((data->state));
      argv[0] = state;
      Nan::MakeCallback(dc, callback, 1, argv);

      if (data->state == webrtc::DataChannelInterface::kClosed) {
        do_shutdown = true;
      }
    } else if (DataChannel::MESSAGE & evt.type) {
      auto data = static_cast<MessageEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(dc->Get(Nan::New("onmessage").ToLocalChecked()));

      Local<Value> argv[1];

      if (data->binary) {
#if NODE_MODULE_VERSION > 0x000B
        Local<v8::ArrayBuffer> array = v8::ArrayBuffer::New(
                v8::Isolate::GetCurrent(), data->message.get(), data->size);
#else
        Local<Object> array = Nan::New(ArrayBufferConstructor)->NewInstance();
        array->SetIndexedPropertiesToExternalArrayData(
            data->message.get(), v8::kExternalByteArray, data->size);
        array->ForceSet(Nan::New("byteLength").ToLocalChecked(), Nan::New<Integer>(static_cast<uint32_t>(data->size)));
#endif
        // NanMakeWeakPersistent(callback, data, &MessageWeakCallback);

        argv[0] = array;
        Nan::MakeCallback(dc, callback, 1, argv);
      } else {
        Local<String> str = Nan::New(data->message.get(), data->size).ToLocalChecked();

        // cleanup message event
        delete data;

        argv[0] = str;
        Nan::MakeCallback(dc, callback, 1, argv);
      }
    }
  }

  if (do_shutdown) {
    uv_close(reinterpret_cast<uv_handle_t*>(&self->async), reinterpret_cast<uv_close_cb>(DataChannel::Shutdown));
  }

  TRACE_END;
}

void DataChannel::Shutdown(uv_async_t* handle) {
  auto self = static_cast<DataChannel*>(handle->data);
  self->Unref();
}

void DataChannel::OnStateChange() {
  TRACE_CALL;
  auto data = new StateEvent(_jingleDataChannel->state());
  QueueEvent(DataChannel::STATE, static_cast<void*>(data));
  if (_jingleDataChannel->state() == webrtc::DataChannelInterface::kClosed) {
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
  auto data = new MessageEvent(&buffer);
  QueueEvent(DataChannel::MESSAGE, static_cast<void*>(data));
  TRACE_END;
}

NAN_METHOD(DataChannel::Send) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.This());

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

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.This());

  if (self->_jingleDataChannel != nullptr) {
    self->_jingleDataChannel->Close();
  }

  TRACE_END;
}

NAN_GETTER(DataChannel::GetBufferedAmount) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  uint64_t buffered_amount = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->buffered_amount()
      : 0;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(buffered_amount));
}

NAN_GETTER(DataChannel::GetId) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  auto id = self->_jingleDataChannel
      ? self->_jingleDataChannel->id()
      : self->_cached_id;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(id));
}

NAN_GETTER(DataChannel::GetLabel) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  std::string label = self->_jingleDataChannel != nullptr
      ? self->_jingleDataChannel->label()
      : self->_cached_label;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(label).ToLocalChecked());
}

NAN_GETTER(DataChannel::GetMaxRetransmits) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  auto max_retransmits = self->_jingleDataChannel
      ? self->_jingleDataChannel->maxRetransmits()
      : self->_cached_max_retransmits;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(max_retransmits));
}

NAN_GETTER(DataChannel::GetOrdered) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  auto ordered = self->_jingleDataChannel
      ? self->_jingleDataChannel->ordered()
      : self->_cached_ordered;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(ordered));
}

NAN_GETTER(DataChannel::GetPriority) {
  TRACE_CALL;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New("high").ToLocalChecked());
}

NAN_GETTER(DataChannel::GetProtocol) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  auto protocol = self->_jingleDataChannel
      ? self->_jingleDataChannel->protocol()
      : self->_cached_protocol;

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(protocol).ToLocalChecked());
}

NAN_GETTER(DataChannel::GetReadyState) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

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

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(self->_binaryType, binaryType, Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(binaryType);
}

NAN_SETTER(DataChannel::SetBinaryType) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<DataChannel>(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, binaryType, BinaryType);

  self->_binaryType = binaryType;

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

#if NODE_MODULE_VERSION < 0x000C
  Local<Object> global = Nan::GetCurrentContext()->Global();
  Local<Value> obj = global->Get(Nan::New("ArrayBuffer").ToLocalChecked());
  ArrayBufferConstructor.Reset(obj.As<Function>());
#endif
}
