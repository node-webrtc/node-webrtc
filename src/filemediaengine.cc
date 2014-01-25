#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "mediastream.h"
#include "mediastreamtrack.h"

using namespace node;
using namespace v8;

Persistent<Function> FileMedia::constructor;

FileMedia::FileMedia(webrtc::FileMediaEngine* fme)
: _internalFileMediaEngine(fme)
{
  fme->Release();
  uv_mutex_init(&lock);
  uv_async_init(uv_default_loop(), &async, Run);

  async.data = this;
}

FileMedia::~FileMedia()
{
}

NAN_METHOD(FileMedia::New) {
  TRACE_CALL;
  NanScope();

  if(!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the FileMedia.");
  }

  v8::Local<v8::External> _fme = v8::Local<v8::External>::Cast(args[0]);
  webrtc::FileMediaInterface* fme = static_cast<webrtc::FileMediaEngine*>(_fme->Value());

  FileMediaEngine* obj = new FileMedia(fme);
  obj->Wrap( args.This() );

  TRACE_END;
  NanReturnValue( args.This() );
}

void FileMedia::QueueEvent(AsyncEventType type, void* data)
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

void FileMedia::Run(uv_async_t* handle, int status)
{
  TRACE_CALL;
  NanScope();
  FileMedia* self = static_cast<FileMedia*>(handle->data);
  v8::Handle<v8::Object> ms = NanObjectWrapHandle(self);

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
    if(FileMedia::PACKET & evt.type)
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(ms->Get(String::New("onpacket")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        callback->Call(ms, 0, argv);
      }
    }
    
    if(MediaStream::MESSAGE & evt.type) 
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(ms->Get(String::New("message")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        callback->Call(ms, 0, argv);
      }
    }
  }
  scope.Close(Undefined());
  TRACE_END;
}

webrtc::FileMediaEngine* FileMedia::GetInterface() {
    return _internalFileMediaEngine;
}

NAN_METHOD(FileMedia::createChannel) {
  TRACE_CALL;
  NanScope();

  FileMedia* self = ObjectWrap::Unwrap<FileMedia>( args.This() );
  v8::String::Utf8Value filepath = v8::Local<v8::String::Utf8Value>::Cast(args[0]);

  *VoiceMediaChannel channel = self->_internalFileMediaEngine->CreateChannel();

  v8::Local<v8::Value> wrapped[1];
  wrapped[0] = v8::External::New(static_cast<void*>(channel));

  TRACE_END;
  NanReturnValue(wrapped);
}

NAN_METHOD(FileMedia::createVideoChannel) {
  TRACE_CALL;
  NanScope();

  FileMedia* self = ObjectWrap::Unwrap<FileMedia>( args.This() );
  v8::String::Utf8Value filepath = v8::Local<v8::String::Utf8Value>::Cast(args[0]);

  *VideoMediaChannel channel = self->_internalFileMediaEngine->CreateVideoChannel();

  v8::Local<v8::Value> wrapped[1];
  wrapped[0] = v8::External::New(static_cast<void*>(channel));

  TRACE_END;
  NanReturnValue(wrapped);
}

NAN_METHOD(FileMedia::setVoiceInputFilename) {
  TRACE_CALL;
  NanScope();

  FileMedia* self = ObjectWrap::Unwrap<FileMedia>( args.This() );
  v8::String::Utf8Value filepath = v8::Local<v8::String::Utf8Value>::Cast(args[0]);

  self->_internalFileMediaEngine->set_voice_input_filename(filepath->ToString());

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(FileMedia::setVoiceOutputFilename) {
  TRACE_CALL;
  NanScope();

  FileMedia* self = ObjectWrap::Unwrap<FileMedia>( args.This() );
  v8::String::Utf8Value filepath = v8::Local<v8::String::Utf8Value>::Cast(args[0]);

  self->_internalFileMediaEngine->set_voice_output_filename(filepath->ToString());

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(FileMedia::setVideoInputFilename) {
  TRACE_CALL;
  NanScope();

  FileMedia* self = ObjectWrap::Unwrap<FileMedia>( args.This() );
  v8::String::Utf8Value filepath = v8::Local<v8::String::Utf8Value>::Cast(args[0]);

  self->_internalFileMediaEngine->set_video_input_filename(filepath->ToString());

  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(FileMedia::setVideoOutputFilename) {
  TRACE_CALL;
  NanScope();

  FileMedia* self = ObjectWrap::Unwrap<FileMedia>( args.This() );
  v8::String::Utf8Value filepath = v8::Local<v8::String::Utf8Value>::Cast(args[0]);

  self->_internalFileMediaEngine->set_video_output_filename(filepath->ToString());

  TRACE_END;
  NanReturnValue(Undefined());
}

void MediaStream::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "FileMedia" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createChannel" ),
    FunctionTemplate::New( getAudioTracks )->GetFunction() );
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createVideoChannel" ),
    FunctionTemplate::New( getVideoTracks )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setVoiceInputFilename" ),
    FunctionTemplate::New( setVoiceInputFilename )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setVoiceOutputFilename" ),
    FunctionTemplate::New( setVoiceOutputFilename )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setVideoInputFilename" ),
    FunctionTemplate::New( setVideoInputFilename )->GetFunction() );
    
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setVideoOutputFilename" ),
    FunctionTemplate::New( setVideoOutputFilename )->GetFunction() );

  NanAssignPersistent(Function, constructor, tpl->GetFunction());
  exports->Set( String::NewSymbol("FileMedia"), tpl->GetFunction() );
}

/* ex: set tabstop=2 expandtab: */
