#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "datachannel.h"

using namespace node;
using namespace v8;

Persistent<Function> DataChannel::constructor;
Persistent<Function> DataChannel::ArrayBufferConstructor;

DataChannelObserver::DataChannelObserver(talk_base::scoped_refptr<webrtc::DataChannelInterface> libjingle_data_channel) {
  TRACE_CALL;
  _libjingle_data_channel = libjingle_data_channel;
  _libjingle_data_channel->RegisterObserver(this);
  TRACE_END;
}

DataChannelObserver::~DataChannelObserver() {
  _libjingle_data_channel->UnregisterObserver();
  _libjingle_data_channel = NULL;
  _js_data_channel = NULL;
}

void DataChannelObserver::OnStateChange()
{
  TRACE_CALL;
  if(_js_data_channel) {
    DataChannel::StateEvent* data = new DataChannel::StateEvent(_libjingle_data_channel->state());
    _js_data_channel->QueueEvent(DataChannel::STATE, static_cast<void*>(data));
  }
  TRACE_END;
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer)
{
  TRACE_CALL;
  if(_js_data_channel) {
    DataChannel::MessageEvent* data = new DataChannel::MessageEvent(&buffer);
    _js_data_channel->QueueEvent(DataChannel::MESSAGE, static_cast<void*>(data));
  }
  TRACE_END;
}

talk_base::scoped_refptr<webrtc::DataChannelInterface> DataChannelObserver::GetLibjingleDataChannel() {
  return _libjingle_data_channel;
}

void DataChannelObserver::SetJsDataChannel(DataChannel* js_data_channel) {
  _js_data_channel = js_data_channel;
}

DataChannel::DataChannel(DataChannelObserver* observer)
: loop(uv_default_loop()),
  _observer(observer),
  _binaryType(DataChannel::ARRAY_BUFFER)
{
  uv_mutex_init(&lock);
  uv_async_init(loop, &async, Run);

  observer->SetJsDataChannel(this);
  _libjingle_data_channel = observer->GetLibjingleDataChannel();

  async.data = this;
}

DataChannel::~DataChannel()
{
  TRACE_CALL;
  TRACE_END;
}

NAN_METHOD(DataChannel::New) {
  TRACE_CALL;
  NanScope();

  if(!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the DataChannel.");
  }

  v8::Local<v8::External> _observer = v8::Local<v8::External>::Cast(args[0]);
  DataChannelObserver* observer = static_cast<DataChannelObserver*>(_observer->Value());

  DataChannel* obj = new DataChannel(observer);
  obj->Wrap( args.This() );
  V8::AdjustAmountOfExternalAllocatedMemory(1024 * 1024);

  TRACE_END;
  NanReturnValue( args.This() );
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

void DataChannel::Run(uv_async_t* handle, int status)
{
  NanScope();
  DataChannel* self = static_cast<DataChannel*>(handle->data);
  TRACE_CALL_P((uintptr_t)self);
  v8::Handle<v8::Object> dc = NanObjectWrapHandle(self);
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
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(String::New("onerror")));
      v8::Local<v8::Value> argv[1];
      argv[0] = Exception::Error(String::New(data->msg.c_str()));
      callback->Call(dc, 1, argv);
    } else if(DataChannel::STATE & evt.type)
    {
      StateEvent* data = static_cast<StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(String::New("onstatechange")));
      v8::Local<v8::Value> argv[1];
      Local<Integer> state = Uint32::New(data->state);
      argv[0] = state;
      callback->Call(dc, 1, argv);

      if(webrtc::DataChannelInterface::kClosed == self->_libjingle_data_channel->state()) {
        do_shutdown = true;
      }
    } else if(DataChannel::MESSAGE & evt.type)
    {
      MessageEvent* data = static_cast<MessageEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(String::New("onmessage")));

      v8::Local<v8::Value> argv[1];

      if(data->binary) {
        Local<Object> array = NanPersistentToLocal(ArrayBufferConstructor)->NewInstance();
        array->SetIndexedPropertiesToExternalArrayData(data->message, v8::kExternalByteArray, data->size);
        Local<String> byteLenghtKey = String::New("byteLength");
        Local<Integer> byteLengthValue = Uint32::New(data->size);
        array->ForceSet(byteLenghtKey, byteLengthValue);
        V8::AdjustAmountOfExternalAllocatedMemory(data->size);

        argv[0] = array;
        callback->Call(dc, 1, argv);
      } else {
        Local<String> str = String::New(data->message, data->size);

        argv[0] = str;
        callback->Call(dc, 1, argv);
      }
    }
    // FIXME: delete event
  }

  if(do_shutdown) {
    uv_close((uv_handle_t*)(&self->async), NULL);
  }

  TRACE_END;
}

NAN_METHOD(DataChannel::Send) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );

  if(args[0]->IsString()) {
    v8::Local<v8::String> str = v8::Local<v8::String>::Cast(args[0]);
    std::string data = *v8::String::Utf8Value(str);

    webrtc::DataBuffer buffer(data);
    self->_libjingle_data_channel->Send(buffer);
  } else {
    v8::Local<v8::Object> arraybuffer = v8::Local<v8::Object>::Cast(args[0]);
    void* data = arraybuffer->GetIndexedPropertiesExternalArrayData();
    uint32_t data_len = arraybuffer->GetIndexedPropertiesExternalArrayDataLength();

    talk_base::Buffer buffer(data, data_len);
    webrtc::DataBuffer data_buffer(buffer, true);
    self->_libjingle_data_channel->Send(data_buffer);
  }

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(DataChannel::Close) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );
  self->_libjingle_data_channel->Close();

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(DataChannel::Shutdown) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );
  if(!uv_is_closing((uv_handle_t*)(&self->async)))
    uv_close((uv_handle_t*)(&self->async), NULL);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_GETTER(DataChannel::GetLabel) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.Holder() );

  std::string label = self->_libjingle_data_channel->label();

  TRACE_END;
  NanReturnValue(String::New(label.c_str()));
}

NAN_GETTER(DataChannel::GetReadyState) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.Holder() );

  webrtc::DataChannelInterface::DataState state = self->_libjingle_data_channel->state();

  TRACE_END;
  NanReturnValue(Number::New(static_cast<uint32_t>(state)));
}

NAN_GETTER(DataChannel::GetBinaryType) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.Holder() );

  TRACE_END;
  NanReturnValue(Number::New(static_cast<uint32_t>(self->_binaryType)));
}

NAN_SETTER(DataChannel::SetBinaryType) {
  TRACE_CALL;

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.Holder() );
  self->_binaryType = static_cast<BinaryType>(value->Uint32Value());

  TRACE_END;
}

NAN_SETTER(DataChannel::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void DataChannel::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "DataChannel" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "close" ),
    FunctionTemplate::New( Close )->GetFunction() );
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "shutdown" ),
    FunctionTemplate::New( Shutdown )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "send" ),
    FunctionTemplate::New( Send )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(String::New("label"), GetLabel, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("binaryType"), GetBinaryType, SetBinaryType);
  tpl->InstanceTemplate()->SetAccessor(String::New("readyState"), GetReadyState, ReadOnly);

  NanAssignPersistent(Function, constructor, tpl->GetFunction());
  exports->Set( String::NewSymbol("DataChannel"), tpl->GetFunction() );

  v8::Local<v8::Object> global = v8::Context::GetCurrent()->Global();
  v8::Local<v8::Value> obj = global->Get(v8::String::New("ArrayBuffer"));
  NanAssignPersistent(Function, ArrayBufferConstructor, obj.As<Function>());
}
