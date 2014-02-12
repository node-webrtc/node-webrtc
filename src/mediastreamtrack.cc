#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "mediastreamtrack.h"

using namespace node;
using namespace v8;

Persistent<Function> MediaStreamTrack::constructor;

MediaStreamTrack::MediaStreamTrack(webrtc::MediaStreamTrackInterface* msti)
: _internalMediaStreamTrack(msti)
{
  msti->Release();
  _muted = false;
  _live = _internalMediaStreamTrack->state() == webrtc::MediaStreamTrackInterface::kLive;
  uv_mutex_init(&lock);
  uv_async_init(uv_default_loop(), &async, Run);

  async.data = this;
}

MediaStreamTrack::~MediaStreamTrack()
{
    
}

NAN_METHOD(MediaStreamTrack::New) {
  TRACE_CALL;
  NanScope();

  if(!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the MediaStreamTrack.");
  }

  v8::Local<v8::External> _msti = v8::Local<v8::External>::Cast(args[0]);
  webrtc::MediaStreamTrackInterface* msti = static_cast<webrtc::MediaStreamTrackInterface*>(_msti->Value());

  MediaStreamTrack* obj = new MediaStreamTrack(msti);
  msti->RegisterObserver(obj);
  obj->Wrap( args.This() );

  TRACE_END;
  NanReturnValue( args.This() );
}

void MediaStreamTrack::QueueEvent(AsyncEventType type, void* data)
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

void MediaStreamTrack::Run(uv_async_t* handle, int status)
{
  TRACE_CALL;
  NanScope();
  MediaStreamTrack* self = static_cast<MediaStreamTrack*>(handle->data);
  v8::Handle<v8::Object> mst = NanObjectWrapHandle(self);

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
    if(MediaStreamTrack::CHANGE & evt.type)
    {
      // find current state and send appropriate events
      bool live = self->_internalMediaStreamTrack->state() == webrtc::MediaStreamTrackInterface::kLive;
      if (self->_live != live)
      {
        if (!live)
          evt.type = MediaStreamTrack::ENDED;
        else
          evt.type = MediaStreamTrack::STARTED;
        self->_live = live;
      }
    }
    if(MediaStreamTrack::MUTE & evt.type) 
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(mst->Get(String::New("onmute")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        callback->Call(mst, 0, argv);
      }
    } else if(MediaStreamTrack::UNMUTE & evt.type) 
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(mst->Get(String::New("onunmute")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        callback->Call(mst, 0, argv);
      }
    }
    if(MediaStreamTrack::STARTED & evt.type) 
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(mst->Get(String::New("onstarted")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        callback->Call(mst, 0, argv);
      }
    }
    if(MediaStreamTrack::ENDED & evt.type) 
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(mst->Get(String::New("onended")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        callback->Call(mst, 0, argv);
      }
    }
    
  }
  scope.Close(Undefined());
  TRACE_END;
}

void MediaStreamTrack::OnChanged()
{
  TRACE_CALL;
  QueueEvent(MediaStreamTrack::CHANGE, static_cast<void*>(NULL));
  TRACE_END;
}

webrtc::MediaStreamTrackInterface* MediaStreamTrack::GetInterface() {
    return _internalMediaStreamTrack;
}

NAN_METHOD(MediaStreamTrack::clone) {
  TRACE_CALL;
  NanScope();
  // todo: implement
  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_METHOD(MediaStreamTrack::stop) {
  TRACE_CALL;
  NanScope();
  // todo: implement
  TRACE_END;
  NanReturnValue(Undefined());
}

NAN_GETTER(MediaStreamTrack::GetId) {
  TRACE_CALL;
  NanScope();

  MediaStreamTrack* self = ObjectWrap::Unwrap<MediaStreamTrack>( args.Holder() );
  std::string id = self->_internalMediaStreamTrack->id();

  TRACE_END;
  NanReturnValue(String::New(id.c_str()));
}

NAN_GETTER(MediaStreamTrack::GetLabel) {
  TRACE_CALL;
  NanScope();

  MediaStreamTrack* self = ObjectWrap::Unwrap<MediaStreamTrack>( args.Holder() );
  std::string label = self->_internalMediaStreamTrack->id();

  TRACE_END;
  NanReturnValue(String::New(label.c_str()));
}

NAN_GETTER(MediaStreamTrack::GetKind) {
  TRACE_CALL;
  NanScope();

  MediaStreamTrack* self = ObjectWrap::Unwrap<MediaStreamTrack>( args.Holder() );
  std::string kind = self->_internalMediaStreamTrack->kind();

  TRACE_END;
  NanReturnValue(String::New(kind.c_str()));
}

NAN_GETTER(MediaStreamTrack::GetEnabled) {
  TRACE_CALL;
  NanScope();

  MediaStreamTrack* self = ObjectWrap::Unwrap<MediaStreamTrack>( args.Holder() );
  bool enabled = self->_internalMediaStreamTrack->enabled();

  TRACE_END;
  NanReturnValue(Boolean::New(enabled));
}

NAN_GETTER(MediaStreamTrack::GetMuted) {
  TRACE_CALL;
  NanScope();
  TRACE_END;
  NanReturnValue(Boolean::New(false));
}

NAN_GETTER(MediaStreamTrack::GetReadOnly) {
  TRACE_CALL;
  NanScope();
  TRACE_END;
  NanReturnValue(Boolean::New(false));
}

NAN_GETTER(MediaStreamTrack::GetRemote) {
  TRACE_CALL;
  NanScope();
  TRACE_END;
  NanReturnValue(Boolean::New(false));
}

NAN_GETTER(MediaStreamTrack::GetReadyState) {
  TRACE_CALL;
  NanScope();

  MediaStreamTrack* self = ObjectWrap::Unwrap<MediaStreamTrack>( args.Holder() );

  webrtc::MediaStreamTrackInterface::TrackState state = self->_internalMediaStreamTrack->state();

  TRACE_END;
  NanReturnValue(Number::New(static_cast<uint32_t>(state)));
}

NAN_SETTER(MediaStreamTrack::SetEnabled) {
  TRACE_CALL;
  NanScope();

  MediaStreamTrack* self = ObjectWrap::Unwrap<MediaStreamTrack>( args.Holder() );
  self->_internalMediaStreamTrack->set_enabled(value->ToBoolean()->Value());
  
  TRACE_END;
}

NAN_SETTER(MediaStreamTrack::ReadOnly) {
  INFO("MediaStreamTrack::ReadOnly");
}

void MediaStreamTrack::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "MediaStreamTrack" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
    
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "clone" ),
    FunctionTemplate::New( clone )->GetFunction() );
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "stop" ),
    FunctionTemplate::New( stop )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(String::New("id"), GetId, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("kind"), GetKind, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("label"), GetLabel, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("enabled"), GetEnabled, SetEnabled);
  tpl->InstanceTemplate()->SetAccessor(String::New("muted"), GetMuted, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("_readonly"), GetReadOnly, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("remote"), GetRemote, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("readyState"), GetReadyState, ReadOnly);

  NanAssignPersistent(Function, constructor, tpl->GetFunction());
  exports->Set( String::NewSymbol("MediaStreamTrack"), tpl->GetFunction() );
}
