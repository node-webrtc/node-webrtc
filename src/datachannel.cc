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
: _internalDataChannel(dci),
  _binaryType(DataChannel::BLOB)
{
  dci->Release();
  uv_mutex_init(&lock);
  uv_async_init(uv_default_loop(), &async, Run);

  async.data = this;
}

DataChannel::~DataChannel()
{

}

Handle<Value> DataChannel::New( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  if(!args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
          String::New("Use the new operator to construct the DataChannel.")));
  }

  v8::Local<v8::External> _dci = v8::Local<v8::External>::Cast(args[0]);
  webrtc::DataChannelInterface* dci = static_cast<webrtc::DataChannelInterface*>(_dci->Value());

  DataChannel* obj = new DataChannel(dci);
  dci->RegisterObserver(obj);
  obj->Wrap( args.This() );

  TRACE_END;
  return scope.Close( args.This() );
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
  HandleScope scope;
  DataChannel* self = static_cast<DataChannel*>(handle->data);
  v8::Persistent<v8::Object> dc = self->handle_;

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
    } else if(DataChannel::MESSAGE & evt.type)
    {
      MessageEvent* data = static_cast<MessageEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(dc->Get(String::New("onmessage")));

      Local<Object> array = ArrayBufferConstructor->NewInstance();
      array->SetIndexedPropertiesToExternalArrayData(data->message, v8::kExternalByteArray, data->size);
      Local<String> byteLenghtKey = String::New("byteLength");
      Local<Integer> byteLengthValue = Uint32::New(data->size);
      array->ForceSet(byteLenghtKey, byteLengthValue);
      V8::AdjustAmountOfExternalAllocatedMemory(data->size);

      v8::Local<v8::Value> argv[1];
      argv[0] = array;
      callback->Call(dc, 1, argv);
    }
  }
  scope.Close(Undefined());
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

Handle<Value> DataChannel::Send( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> DataChannel::Close( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( args.This() );

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> DataChannel::GetLabel( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( info.Holder() );

  std::string label = self->_internalDataChannel->label();

  TRACE_END;
  return scope.Close(String::New(label.c_str()));
}

Handle<Value> DataChannel::GetReadyState( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( info.Holder() );

  webrtc::DataChannelInterface::DataState state = self->_internalDataChannel->state();

  TRACE_END;
  return scope.Close(Number::New(static_cast<uint32_t>(state)));
}

Handle<Value> DataChannel::GetBinaryType( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( info.Holder() );

  TRACE_END;
  return scope.Close(Number::New(static_cast<uint32_t>(self->_binaryType)));
}

void DataChannel::SetBinaryType( Local<String> property, Local<Value> value, const AccessorInfo& info ) {
  TRACE_CALL

  DataChannel* self = ObjectWrap::Unwrap<DataChannel>( info.Holder() );
  self->_binaryType = static_cast<BinaryType>(value->Uint32Value());

  TRACE_END
}

void DataChannel::ReadOnly( Local<String> property, Local<Value> value, const AccessorInfo& info ) {
  INFO("PeerConnection::ReadOnly");
}

void DataChannel::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "DataChannel" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "close" ),
    FunctionTemplate::New( Close )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "send" ),
    FunctionTemplate::New( Send )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(String::New("label"), GetLabel, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("binaryType"), GetBinaryType, SetBinaryType);
  tpl->InstanceTemplate()->SetAccessor(String::New("readyState"), GetReadyState, ReadOnly);

  constructor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("DataChannel"), constructor );

  v8::Local<v8::Object> global = v8::Context::GetCurrent()->Global();
  v8::Local<v8::Value> obj = global->Get(v8::String::New("ArrayBuffer"));
  ArrayBufferConstructor = Persistent<Function>::New(obj.As<Function>());
}