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

DataChannel::DataChannel(webrtc::DataChannelInterface* dci)
: loop(uv_default_loop()),
  _internalDataChannel(dci),
  _binaryType(DataChannel::BLOB)
{
  dci->Release();
  uv_mutex_init(&lock);
  uv_async_init(loop, &async, Run);

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

  v8::Local<v8::External> _dci = v8::Local<v8::External>::Cast(args[0]);
  webrtc::DataChannelInterface* dci = static_cast<webrtc::DataChannelInterface*>(_dci->Value());

  DataChannel* obj = new DataChannel(dci);
  dci->RegisterObserver(obj);
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
  TRACE_CALL;
  NanScope();
  DataChannel* self = static_cast<DataChannel*>(handle->data);
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
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(String::New("onstatechange")));
      v8::Local<v8::Value> argv[0];
      callback->Call(dc, 0, argv);

      if(webrtc::DataChannelInterface::kClosed == self->_internalDataChannel->state()) {
        do_shutdown = true;
      }
    } else if(DataChannel::MESSAGE & evt.type)
    {
      MessageEvent* data = static_cast<MessageEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(String::New("onmessage")));

      Local<Object> array = NanPersistentToLocal(ArrayBufferConstructor)->NewInstance();
      array->SetIndexedPropertiesToExternalArrayData(data->message, v8::kExternalByteArray, data->size);
      Local<String> byteLenghtKey = String::New("byteLength");
      Local<Integer> byteLengthValue = Uint32::New(data->size);
      array->ForceSet(byteLenghtKey, byteLengthValue);
      V8::AdjustAmountOfExternalAllocatedMemory(data->size);

      v8::Local<v8::Value> argv[1];
      argv[0] = array;
      callback->Call(dc, 1, argv);
    }
    // FIXME: delete event
  }

  if(do_shutdown) {
    uv_close((uv_handle_t*)(&self->async), NULL);
  }

  TRACE_END;
}

void DataChannel::OnStateChange()
{
  TRACE_CALL;
  QueueEvent(DataChannel::STATE, static_cast<void*>(NULL));
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
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );
  v8::Local<v8::Object> arraybuffer = v8::Local<v8::Object>::Cast(args[0]);

  void* data = arraybuffer->GetIndexedPropertiesExternalArrayData();
  uint32_t data_len = arraybuffer->GetIndexedPropertiesExternalArrayDataLength();

  talk_base::Buffer buffer(data, data_len);
  webrtc::DataBuffer data_buffer(buffer, true);
  self->_internalDataChannel->Send(data_buffer);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(DataChannel::Close) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );
  self->_internalDataChannel->Close();

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(DataChannel::Shutdown) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );
  uv_close((uv_handle_t*)(&self->async), NULL);

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_GETTER(DataChannel::GetLabel) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.Holder() );

  std::string label = self->_internalDataChannel->label();

  TRACE_END;
  NanReturnValue(String::New(label.c_str()));
}

NAN_GETTER(DataChannel::GetReadyState) {
  TRACE_CALL;
  NanScope();

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.Holder() );

  webrtc::DataChannelInterface::DataState state = self->_internalDataChannel->state();

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
