/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "mediastreamtrack.h"

#include "converters/webrtc.h"

using node_webrtc::MediaStreamTrack;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> MediaStreamTrack::constructor;

MediaStreamTrack::MediaStreamTrack(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>&& track)
  : Nan::AsyncResource("MediaStreamTrack")
  , PromiseFulfillingEventLoop(*this)
  , _factory(std::move(factory))
  , _track(std::move(track)) {
  _track->RegisterObserver(this);
}

MediaStreamTrack::~MediaStreamTrack() {
  _track->UnregisterObserver(this);
}

NAN_METHOD(MediaStreamTrack::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a MediaStreamTrack");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(Local<External>::Cast(info[0])->Value());
  auto track = *static_cast<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>*>(Local<External>::Cast(info[1])->Value());

  auto obj = new MediaStreamTrack(std::move(factory), std::move(track));
  obj->Wrap(info.This());
  obj->Ref();

  info.GetReturnValue().Set(info.This());
}

void MediaStreamTrack::OnChanged() {
  if (this->_track->state() == webrtc::MediaStreamTrackInterface::TrackState::kEnded) {
    Stop();
  }
}

void MediaStreamTrack::DidStop() {
  Unref();
}

NAN_GETTER(MediaStreamTrack::GetEnabled) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->enabled()));
}

NAN_GETTER(MediaStreamTrack::GetId) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->id()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetKind) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->kind()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetReadyState) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<MediaStreamTrack>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_track->state(), result, std::string);
  info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
}

void MediaStreamTrack::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("MediaStreamTrack").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("enabled").ToLocalChecked(), GetEnabled, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("kind").ToLocalChecked(), GetKind, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState, nullptr);
  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("MediaStreamTrack").ToLocalChecked(), tpl->GetFunction());
}
