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

Nan::Persistent<Function> MediaStream::constructor;

MediaStream::MediaStream(webrtc::MediaStreamInterface* msi)
: _internalMediaStream(msi)
{
  msi->Release();
  _inactive = !IsMediaStreamActive();
  uv_mutex_init(&lock);
  uv_async_init(uv_default_loop(), &async, Run);

  async.data = this;
}

MediaStream::~MediaStream()
{

}

NAN_METHOD(MediaStream::New) {
  TRACE_CALL;
  Nan::HandleScope scope;

  if(!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the MediaStream.");
  }

  v8::Local<v8::External> _msi = v8::Local<v8::External>::Cast(info[0]);
  webrtc::MediaStreamInterface* msi = static_cast<webrtc::MediaStreamInterface*>(_msi->Value());

  MediaStream* obj = new MediaStream(msi);
  msi->RegisterObserver(obj);
  obj->Wrap( info.This() );

  TRACE_END;
  info.GetReturnValue().Set( info.This() );
}

void MediaStream::QueueEvent(AsyncEventType type, void* data)
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

void MediaStream::Run(uv_async_t* handle, int status)
{
  TRACE_CALL;
  Nan::HandleScope scope;
  MediaStream* self = static_cast<MediaStream*>(handle->data);
  v8::Handle<v8::Object> ms = self->handle();

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
    if(MediaStream::CHANGE & evt.type)
    {
      // find current state and send appropriate events
      bool inactive = !self->IsMediaStreamActive();
      if (self->_inactive != inactive)
      {
        if (!inactive)
          evt.type = MediaStream::ACTIVE;
        else
          evt.type = MediaStream::INACTIVE;
        self->_inactive = inactive;
      }
    }

    if(MediaStream::ACTIVE & evt.type)
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(ms->Get(String::New("onactive")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        Nan::MakeCallback(ms, callback, 0, argv);
      }
    } else if(MediaStream::INACTIVE & evt.type)
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(ms->Get(String::New("oninactive")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[0];
        Nan::MakeCallback(ms, callback, 0, argv);
      }
    }
    if(MediaStream::ADDTRACK & evt.type)
    {
      webrtc::MediaStreamTrackInterface* msti = static_cast<webrtc::MediaStreamTrackInterface*>(evt.data);
      v8::Local<v8::Value> cargv[1];
      cargv[0] = v8::External::New(static_cast<void*>(msti));
      v8::Local<v8::Value> mst = NanPersistentToLocal(MediaStreamTrack::constructor)->NewInstance(1, cargv);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(ms->Get(String::New("onaddtrack")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = mst;
        Nan::MakeCallback(ms, callback, 1, argv);
      }
    }
    if(MediaStream::REMOVETRACK & evt.type)
    {
      webrtc::MediaStreamTrackInterface* msti = static_cast<webrtc::MediaStreamTrackInterface*>(evt.data);
      v8::Local<v8::Value> cargv[1];
      cargv[0] = v8::External::New(static_cast<void*>(msti));
      v8::Local<v8::Value> mst = NanPersistentToLocal(MediaStreamTrack::constructor)->NewInstance(1, cargv);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(ms->Get(String::New("onremovetrack")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = mst;
        Nan::MakeCallback(ms, callback, 1, argv);
      }
    }
  }
  scope.Close(Undefined());
  TRACE_END;
}

bool MediaStream::IsMediaStreamActive()
{
  webrtc::VideoTrackVector videoTracks = _internalMediaStream->GetVideoTracks();
  for (webrtc::VideoTrackVector::iterator track = videoTracks.begin(); track != videoTracks.end(); track++)
  {
    if ((*track)->state() == webrtc::MediaStreamTrackInterface::kLive)
      return true;
  }

  webrtc::AudioTrackVector audioTracks = _internalMediaStream->GetAudioTracks();
  for (webrtc::AudioTrackVector::iterator track = audioTracks.begin(); track != audioTracks.end(); track++)
  {
    if ((*track)->state() == webrtc::MediaStreamTrackInterface::kLive)
      return true;
  }

  return false;
}

void MediaStream::OnChanged()
{
  TRACE_CALL;
  QueueEvent(MediaStream::CHANGE, static_cast<void*>(NULL));
  TRACE_END;
}

webrtc::MediaStreamInterface* MediaStream::GetInterface() {
    return _internalMediaStream;
}

NAN_METHOD(MediaStream::getAudioTracks) {
  TRACE_CALL;
  Nan::HandleScope scope;

  MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>( info.Holder() );
  webrtc::AudioTrackVector audioTracks = self->_internalMediaStream->GetAudioTracks();

  v8::Local<v8::Array> array = v8::Array::New(audioTracks.size());
  int index = 0;
  for (webrtc::AudioTrackVector::iterator track = audioTracks.begin(); track != audioTracks.end(); track++, index++) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = v8::External::New(static_cast<void*>(track->get()));
    array->Set(index, NanPersistentToLocal(MediaStreamTrack::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  info.GetReturnValue().Set(array);
}

NAN_METHOD(MediaStream::getVideoTracks) {
  TRACE_CALL;
  Nan::HandleScope scope;

  MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>( info.Holder() );
  webrtc::VideoTrackVector videoTracks = self->_internalMediaStream->GetVideoTracks();

  v8::Local<v8::Array> array = v8::Array::New(videoTracks.size());
  int index = 0;
  for (webrtc::VideoTrackVector::iterator track = videoTracks.begin(); track != videoTracks.end(); track++, index++) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = v8::External::New(static_cast<void*>(track->get()));
    array->Set(index, NanPersistentToLocal(MediaStreamTrack::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  info.GetReturnValue().Set(array);
}

NAN_METHOD(MediaStream::getTrackById) {
  TRACE_CALL;
  Nan::HandleScope scope;

  MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>( info.Holder() );
  v8::String::Utf8Value param1(info[0]->ToString());
  std::string _id = std::string(*param1);

  talk_base::scoped_refptr<webrtc::MediaStreamTrackInterface> audioTrack = self->_internalMediaStream->FindAudioTrack(_id);
  talk_base::scoped_refptr<webrtc::MediaStreamTrackInterface> videoTrack = self->_internalMediaStream->FindVideoTrack(_id);

  talk_base::scoped_refptr<webrtc::MediaStreamTrackInterface> track = audioTrack.get() ? audioTrack : videoTrack;
  webrtc::MediaStreamTrackInterface* msti = track.get();
  msti->AddRef();

  v8::Local<v8::Value> cargv[1];
  cargv[0] = v8::External::New(static_cast<void*>(msti));
  v8::Local<v8::Value> mst = NanPersistentToLocal(MediaStreamTrack::constructor)->NewInstance(1, cargv);

  TRACE_END;
  info.GetReturnValue().Set(mst);
}

NAN_METHOD(MediaStream::addTrack) {
  TRACE_CALL;
  Nan::HandleScope scope;

  MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>( info.Holder() );
  MediaStreamTrack* _track = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info[0]->ToObject());

  if (_track->GetInterface()->kind() == "audio") {
    self->_internalMediaStream->AddTrack((webrtc::AudioTrackInterface*)_track->GetInterface());
  } else if (_track->GetInterface()->kind() == "video") {
    self->_internalMediaStream->AddTrack((webrtc::VideoTrackInterface*)_track->GetInterface());
  }

  TRACE_END;
  info.GetReturnValue().Set(Undefined());
}

NAN_METHOD(MediaStream::removeTrack) {
  TRACE_CALL;
  Nan::HandleScope scope;

  MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>( info.Holder() );
  MediaStreamTrack* _track = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info[0]->ToObject());

  if (_track->GetInterface()->kind() == "audio") {
    self->_internalMediaStream->RemoveTrack((webrtc::AudioTrackInterface*)_track->GetInterface());
  } else if (_track->GetInterface()->kind() == "video") {
    self->_internalMediaStream->RemoveTrack((webrtc::VideoTrackInterface*)_track->GetInterface());
  }

  TRACE_END;
  info.GetReturnValue().Set(Undefined());
}

NAN_METHOD(MediaStream::clone) {
  TRACE_CALL;
  Nan::HandleScope scope;

  TRACE_END;
  info.GetReturnValue().Set(Undefined());
}

NAN_GETTER(MediaStream::GetId) {
  TRACE_CALL;
  Nan::HandleScope scope;

  MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>( info.Holder() );

  std::string label = self->_internalMediaStream->label();

  TRACE_END;
  info.GetReturnValue().Set(String::New(label.c_str()));
}

NAN_GETTER(MediaStream::IsInactive) {
  TRACE_CALL;
  Nan::HandleScope scope;

  MediaStream* self = Nan::ObjectWrap::Unwrap<MediaStream>( info.Holder() );
  bool inactive = self->_inactive;

  TRACE_END;
  info.GetReturnValue().Set(Boolean::New(inactive));
}

NAN_SETTER(MediaStream::ReadOnly) {
  INFO("MediaStream::ReadOnly");
}


void MediaStream::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "MediaStream" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "getaudiotracks" ),
    FunctionTemplate::New( getAudioTracks )->GetFunction() );
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "getvideotracks" ),
    FunctionTemplate::New( getVideoTracks )->GetFunction() );
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "gettrackbyid" ),
    FunctionTemplate::New( getTrackById )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "addtrack" ),
    FunctionTemplate::New( addTrack )->GetFunction() );
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "removetrack" ),
    FunctionTemplate::New( removeTrack )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "clone" ),
    FunctionTemplate::New( clone )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(String::New("id"), GetId, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("inactive"), IsInactive, ReadOnly);

  Function.Reset(constructor, tpl->GetFunction());
  exports->Set( String::NewSymbol("MediaStream"), tpl->GetFunction() );
}
