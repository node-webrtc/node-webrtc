#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"
#include "common.h"
#include "datachannel.h"

using namespace node_webrtc;

Nan::Persistent<v8::Function> DataChannel::constructor;
#if NODE_MODULE_VERSION < 0x000C
Nan::Persistent<v8::Function> DataChannel::ArrayBufferConstructor;
#endif

DataChannelObserver::DataChannelObserver(rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel) {
  TRACE_CALL;
  uv_mutex_init(&lock);
  _jingleDataChannel = jingleDataChannel;
  _jingleDataChannel->RegisterObserver(this);
  TRACE_END;
}

DataChannelObserver::~DataChannelObserver() {
  _jingleDataChannel = NULL;
}

void DataChannelObserver::OnStateChange()
{
  TRACE_CALL;
  DataChannel::StateEvent* data = new DataChannel::StateEvent(_jingleDataChannel->state());
  QueueEvent(DataChannel::STATE, static_cast<void*>(data));
  TRACE_END;
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer)
{
  TRACE_CALL;
    DataChannel::MessageEvent* data = new DataChannel::MessageEvent(&buffer);
    QueueEvent(DataChannel::MESSAGE, static_cast<void*>(data));
  TRACE_END;
}

void DataChannelObserver::QueueEvent(DataChannel::AsyncEventType type, void* data) {
  TRACE_CALL;
  DataChannel::AsyncEvent evt;
  evt.type = type;
  evt.data = data;
  uv_mutex_lock(&lock);
  _events.push(evt);
  uv_mutex_unlock(&lock);
  TRACE_END;
}

DataChannel::DataChannel(node_webrtc::DataChannelObserver* observer)
: loop(uv_default_loop()),
  _observer(observer),
  _binaryType(DataChannel::ARRAY_BUFFER)
{
  uv_mutex_init(&lock);
  uv_async_init(loop, &async, reinterpret_cast<uv_async_cb>(Run));

  _jingleDataChannel = observer->_jingleDataChannel;
  _jingleDataChannel->RegisterObserver(this);

  async.data = this;

  // Re-queue cached observer events
  while(true) {
    uv_mutex_lock(&observer->lock);
    bool empty = observer->_events.empty();
    if(empty)
    {
      uv_mutex_unlock(&observer->lock);
      break;
    }
    AsyncEvent evt = observer->_events.front();
    observer->_events.pop();
    uv_mutex_unlock(&observer->lock);
    QueueEvent(evt.type, evt.data);
  }

  delete observer;
}

DataChannel::~DataChannel()
{
  TRACE_CALL;
  TRACE_END;
}

NAN_METHOD(DataChannel::New) {
  TRACE_CALL;
  Nan::HandleScope scope;

  if(!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the DataChannel.");
  }

  v8::Local<v8::External> _observer = v8::Local<v8::External>::Cast(info[0]);
  node_webrtc::DataChannelObserver* observer = static_cast<node_webrtc::DataChannelObserver*>(_observer->Value());

  DataChannel* obj = new DataChannel(observer);
  obj->Wrap( info.This() );

  TRACE_END;
  info.GetReturnValue().Set( info.This() );
}

void DataChannel::QueueEvent(AsyncEventType type, void* data)
{
  TRACE_CALL;
  AsyncEvent evt;
  evt.type = type;
  evt.data = data;
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
  parameter->message = NULL;
  delete parameter;
  //Nan::AdjustExternalMemory(-parameter->size);
}*/

void DataChannel::Run(uv_async_t* handle, int status)
{
  Nan::HandleScope scope;
  DataChannel* self = static_cast<DataChannel*>(handle->data);
  TRACE_CALL_P((uintptr_t)self);
  v8::Local<v8::Object> dc = self->handle();
  bool do_shutdown = false;

  while(true)
  {
    uv_mutex_lock(&self->lock);
    bool empty = self->_events.empty();
    if(empty)
    {
      uv_mutex_unlock(&self->lock);
      break;
    }
    AsyncEvent evt = self->_events.front();
    self->_events.pop();
    uv_mutex_unlock(&self->lock);

    TRACE_U("evt.type", evt.type);
    if(DataChannel::ERROR & evt.type)
    {
      DataChannel::ErrorEvent* data = static_cast<DataChannel::ErrorEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(Nan::New("onerror").ToLocalChecked()));
      v8::Local<v8::Value> argv[1];
      argv[0] = Nan::Error(data->msg.c_str());
      Nan::MakeCallback(dc, callback, 1, argv);
    } else if(DataChannel::STATE & evt.type)
    {
      StateEvent* data = static_cast<StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(Nan::New("onstatechange").ToLocalChecked()));
      v8::Local<v8::Value> argv[1];
      v8::Local<v8::Integer> state = Nan::New<v8::Integer>((data->state));
      argv[0] = state;
      Nan::MakeCallback(dc, callback, 1, argv);

      if(self->_jingleDataChannel && webrtc::DataChannelInterface::kClosed == self->_jingleDataChannel->state()) {
        do_shutdown = true;
      }
    } else if(DataChannel::MESSAGE & evt.type)
    {
      MessageEvent* data = static_cast<MessageEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(Nan::New("onmessage").ToLocalChecked()));

      v8::Local<v8::Value> argv[1];

      if(data->binary) {
#if NODE_MODULE_VERSION > 0x000B
        v8::Local<v8::ArrayBuffer> array = v8::ArrayBuffer::New(
            v8::Isolate::GetCurrent(), data->message, data->size);
#else
        v8::Local<v8::Object> array = Nan::New(ArrayBufferConstructor)->NewInstance();
        array->SetIndexedPropertiesToExternalArrayData(
            data->message, v8::kExternalByteArray, data->size);
        array->ForceSet(Nan::New("byteLength").ToLocalChecked(), Nan::New<v8::Integer>(static_cast<uint32_t>(data->size)));
#endif
        //NanMakeWeakPersistent(callback, data, &MessageWeakCallback);

        argv[0] = array;
        Nan::MakeCallback(dc, callback, 1, argv);
      } else {
        v8::Local<v8::String> str = Nan::New(data->message, data->size).ToLocalChecked();

        // cleanup message event
        delete[] data->message;
        data->message = NULL;
        delete data;

        argv[0] = str;
        Nan::MakeCallback(dc, callback, 1, argv);
      }
    }
  }

  if(do_shutdown) {
    uv_close((uv_handle_t*)(&self->async), NULL);
    self->_jingleDataChannel->UnregisterObserver();
    self->_jingleDataChannel = NULL;
  }

  TRACE_END;
}

void DataChannel::OnStateChange()
{
  TRACE_CALL;
  StateEvent* data = new StateEvent(_jingleDataChannel->state());
  QueueEvent(DataChannel::STATE, static_cast<void*>(data));
  TRACE_END;
}

void DataChannel::OnMessage(const webrtc::DataBuffer& buffer)
{
  TRACE_CALL;
  MessageEvent* data = new MessageEvent(&buffer);
  QueueEvent(DataChannel::MESSAGE, static_cast<void*>(data));
  TRACE_END;
}

NAN_METHOD(DataChannel::Send) {
  TRACE_CALL;
  Nan::HandleScope scope;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.This() );

  if(info[0]->IsString()) {
    v8::Local<v8::String> str = v8::Local<v8::String>::Cast(info[0]);
    std::string data = *v8::String::Utf8Value(str);

    webrtc::DataBuffer buffer(data);
    self->_jingleDataChannel->Send(buffer);
  } else {

#if NODE_MINOR_VERSION >= 11 || NODE_MAJOR_VERSION > 0
    v8::Local<v8::ArrayBuffer> arraybuffer;

    if (info[0]->IsArrayBuffer()) {
      arraybuffer = v8::Local<v8::ArrayBuffer>::Cast(info[0]);
    } else {
      v8::Local<v8::ArrayBufferView> view = v8::Local<v8::ArrayBufferView>::Cast(info[0]);
      arraybuffer = view->Buffer();
    }

    v8::ArrayBuffer::Contents content = arraybuffer->Externalize();
    rtc::Buffer buffer(content.Data(), content.ByteLength());

#else
    v8::Local<v8::Object> arraybuffer = v8::Local<v8::Object>::Cast(info[0]);
    void* data = arraybuffer->GetIndexedPropertiesExternalArrayData();
    uint32_t data_len = arraybuffer->GetIndexedPropertiesExternalArrayDataLength();

    rtc::Buffer buffer(data, data_len);

#endif

    webrtc::DataBuffer data_buffer(buffer, true);
    self->_jingleDataChannel->Send(data_buffer);

#if NODE_MINOR_VERSION >= 11 || NODE_MAJOR_VERSION > 0
    arraybuffer->Neuter();
#endif
  }

  TRACE_END;
  return;
}

NAN_METHOD(DataChannel::Close) {
  TRACE_CALL;
  Nan::HandleScope scope;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.This() );
  self->_jingleDataChannel->Close();

  TRACE_END;
  return;
}

NAN_METHOD(DataChannel::Shutdown) {
  TRACE_CALL;
  Nan::HandleScope scope;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.This() );
  if(!uv_is_closing((uv_handle_t*)(&self->async)))
    uv_close((uv_handle_t*)(&self->async), NULL);

  TRACE_END;
  return;
}

NAN_GETTER(DataChannel::GetBufferedAmount) {
  TRACE_CALL;
  Nan::HandleScope scope;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.Holder() );

  uint64_t buffered_amount = self->_jingleDataChannel->buffered_amount();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<v8::Number>(buffered_amount));
}

NAN_GETTER(DataChannel::GetLabel) {
  TRACE_CALL;
  Nan::HandleScope scope;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.Holder() );

  std::string label = self->_jingleDataChannel->label();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(label).ToLocalChecked());
}

NAN_GETTER(DataChannel::GetReadyState) {
  TRACE_CALL;
  Nan::HandleScope scope;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.Holder() );

  webrtc::DataChannelInterface::DataState state = self->_jingleDataChannel->state();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<v8::Number>(static_cast<uint32_t>(state)));
}

NAN_GETTER(DataChannel::GetBinaryType) {
  TRACE_CALL;
  Nan::HandleScope scope;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.Holder() );

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<v8::Number>(static_cast<uint32_t>(self->_binaryType)));
}

NAN_SETTER(DataChannel::SetBinaryType) {
  TRACE_CALL;

  DataChannel* self = Nan::ObjectWrap::Unwrap<DataChannel>( info.Holder() );
  self->_binaryType = static_cast<BinaryType>(value->Uint32Value());

  TRACE_END;
}

NAN_SETTER(DataChannel::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void DataChannel::Init( v8::Handle<v8::Object> exports ) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>( New );
  tpl->SetClassName( Nan::New( "DataChannel" ).ToLocalChecked() );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set( Nan::New( "close" ).ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>( Close )->GetFunction() );
  tpl->PrototypeTemplate()->Set( Nan::New( "shutdown" ).ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>( Shutdown )->GetFunction() );

  tpl->PrototypeTemplate()->Set( Nan::New( "send" ).ToLocalChecked(),
    Nan::New<v8::FunctionTemplate>( Send )->GetFunction() );

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("bufferedAmount").ToLocalChecked(), GetBufferedAmount, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("label").ToLocalChecked(), GetLabel, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("binaryType").ToLocalChecked(), GetBinaryType, SetBinaryType);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set( Nan::New("DataChannel").ToLocalChecked(), tpl->GetFunction() );

  v8::Local<v8::Object> global = Nan::GetCurrentContext()->Global();
#if NODE_MODULE_VERSION < 0x000C
  v8::Local<v8::Value> obj = global->Get(Nan::New("ArrayBuffer").ToLocalChecked());
  ArrayBufferConstructor.Reset(obj.As<v8::Function>());
#endif
}
