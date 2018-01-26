#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "common.h"
#include "mediastreamtrack.h"

using node_webrtc::MediaStreamTrack;
using namespace node;
using namespace v8;

Nan::Persistent<Function> MediaStreamTrack::constructor;

MediaStreamTrack::MediaStreamTrack(webrtc::MediaStreamTrackInterface* msti)
: loop(uv_default_loop()),
  _internalMediaStreamTrack(msti)
{
  msti->Release();
  _muted = false;
  _live = _internalMediaStreamTrack->state() == webrtc::MediaStreamTrackInterface::kLive;

  uv_mutex_init(&lock);
  uv_async_init(loop, &async, reinterpret_cast<uv_async_cb>(Run));

  async.data = this;
}

MediaStreamTrack::~MediaStreamTrack()
{

}

NAN_METHOD(MediaStreamTrack::New) 
{
  TRACE_CALL;
  Nan::HandleScope scope;

  if(!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the MediaStreamTrack.");
  }

  v8::Local<v8::External> _msti = v8::Local<v8::External>::Cast(info[0]);
  webrtc::MediaStreamTrackInterface* msti = static_cast<webrtc::MediaStreamTrackInterface*>(_msti->Value());

  MediaStreamTrack* obj = new MediaStreamTrack(msti);
  msti->RegisterObserver(obj);
  obj->Wrap( info.This() );

  TRACE_END;
  info.GetReturnValue().Set( info.This() );
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

  // todo: implement

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(MediaStreamTrack::stop) {
  TRACE_CALL;

  // todo: implement

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(MediaStreamTrack::GetId) {
  TRACE_CALL;

  MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>( info.Holder() );
  std::string id = self->_internalMediaStreamTrack->id();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(id.c_str()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetLabel) {
  TRACE_CALL;

  MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>( info.Holder() );
  std::string label = self->_internalMediaStreamTrack->id();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(label.c_str()).ToLocalChecked());
}


NAN_GETTER(MediaStreamTrack::GetKind) {
  TRACE_CALL;

  MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>( info.Holder() );
  std::string kind = self->_internalMediaStreamTrack->kind();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(kind.c_str()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetEnabled) {
  TRACE_CALL;

  MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>( info.Holder() );
  bool enabled = self->_internalMediaStreamTrack->enabled();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(enabled));
}

NAN_GETTER(MediaStreamTrack::GetMuted) {
  TRACE_CALL;

  // todo: implement

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(false));
}

NAN_GETTER(MediaStreamTrack::GetReadOnly) {
  TRACE_CALL;

  // todo: implement

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(false));
}

NAN_GETTER(MediaStreamTrack::GetRemote) {
  TRACE_CALL;

  // todo: implement

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(false));
}

NAN_GETTER(MediaStreamTrack::GetReadyState) {
  TRACE_CALL;

  MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>( info.Holder() );
  webrtc::MediaStreamTrackInterface::TrackState state = self->_internalMediaStreamTrack->state();

  TRACE_END;
  info.GetReturnValue().Set(Nan::New(static_cast<uint32_t>(state)));
}

NAN_SETTER(MediaStreamTrack::SetEnabled) {
  TRACE_CALL;

  MediaStreamTrack* self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>( info.Holder() );
  self->_internalMediaStreamTrack->set_enabled(value->BooleanValue());

  TRACE_END;
}


NAN_SETTER(MediaStreamTrack::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void MediaStreamTrack::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("MediaStreamTrack").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "clone", clone);
  Nan::SetPrototypeMethod(tpl, "stop", stop);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("label").ToLocalChecked(), GetLabel, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("kind").ToLocalChecked(), GetKind, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("enabled").ToLocalChecked(), GetEnabled, SetEnabled);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("muted").ToLocalChecked(), GetMuted, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readonly").ToLocalChecked(), GetReadOnly, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remote").ToLocalChecked(), GetRemote, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readystate").ToLocalChecked(), GetReadyState, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("MediaStreamTrack").ToLocalChecked(), tpl->GetFunction());
}
