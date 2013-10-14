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

DataChannel::DataChannel(webrtc::DataChannelInterface* dci)
: _internalDataChannel(dci)
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

    }

  }

  TRACE_END;
}

void DataChannel::OnStateChange()
{

}

void DataChannel::OnMessage(const webrtc::DataBuffer& buffer)
{

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

void DataChannel::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "DataChannel" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "close" ),
    FunctionTemplate::New( Close )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "send" ),
    FunctionTemplate::New( Send )->GetFunction() );

  constructor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("DataChannel"), constructor );
}