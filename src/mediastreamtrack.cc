/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "mediastreamtrack.h"

#include "webrtc/base/helpers.h"

#include "converters/webrtc.h"

using node_webrtc::AsyncObjectWrapWithLoop;
using node_webrtc::BidiMap;
using node_webrtc::MediaStreamTrack;
using node_webrtc::PeerConnectionFactory;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> MediaStreamTrack::constructor;

BidiMap<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>, MediaStreamTrack*> MediaStreamTrack::_tracks;

MediaStreamTrack::MediaStreamTrack(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>&& track)
  : AsyncObjectWrapWithLoop("MediaStreamTrack", *this)
  , _factory(std::move(factory))
  , _track(std::move(track)) {
  _track->RegisterObserver(this);
}

MediaStreamTrack::~MediaStreamTrack() {
  _track->UnregisterObserver(this);
  MediaStreamTrack::Release(this);
}

NAN_METHOD(MediaStreamTrack::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a MediaStreamTrack");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(Local<External>::Cast(info[0])->Value());
  auto track = *static_cast<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>*>(Local<External>::Cast(info[1])->Value());

  auto obj = new MediaStreamTrack(std::move(factory), std::move(track));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

void MediaStreamTrack::OnChanged() {
  if (this->_track->state() == webrtc::MediaStreamTrackInterface::TrackState::kEnded) {
    Stop();
  }
}

NAN_GETTER(MediaStreamTrack::GetEnabled) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->enabled()));
}

NAN_GETTER(MediaStreamTrack::GetId) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->id()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetKind) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->kind()).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetReadyState) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_track->state(), result, std::string);
  info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
}

NAN_METHOD(MediaStreamTrack::Clone) {
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  auto label = rtc::CreateRandomUuid();
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> clonedTrack = nullptr;
  if (self->_track->kind() == self->_track->kAudioKind) {
    auto audioTrack = static_cast<webrtc::AudioTrackInterface*>(self->_track.get());
    clonedTrack = self->_factory->factory()->CreateAudioTrack(label, audioTrack->GetSource());
  } else {
    auto videoTrack = static_cast<webrtc::VideoTrackInterface*>(self->_track.get());
    clonedTrack = self->_factory->factory()->CreateVideoTrack(label, videoTrack->GetSource());
  }
  auto clonedMediaStreamTrack = MediaStreamTrack::GetOrCreate(self->_factory, clonedTrack);
  if (self->_track->state() != webrtc::MediaStreamTrackInterface::TrackState::kLive) {
    clonedMediaStreamTrack->Stop();
  }
  info.GetReturnValue().Set(clonedMediaStreamTrack->ToObject());
}

NAN_METHOD(MediaStreamTrack::JsStop) {
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  self->Stop();
}

MediaStreamTrack* MediaStreamTrack::GetOrCreate(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track) {
  return _tracks.computeIfAbsent(track, [&factory, &track]() {
    Nan::HandleScope scope;
    Local<Value> cargv[2];
    cargv[0] = Nan::New<External>(static_cast<void*>(&factory));
    cargv[1] = Nan::New<External>(static_cast<void*>(&track));
    auto mediaStreamTrack = Nan::NewInstance(Nan::New(MediaStreamTrack::constructor), 2, cargv).ToLocalChecked();
    return AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(mediaStreamTrack);
  });
}

void MediaStreamTrack::Release(MediaStreamTrack* track) {
  _tracks.reverseRemove(track);
}

void MediaStreamTrack::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("MediaStreamTrack").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("enabled").ToLocalChecked(), GetEnabled, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("kind").ToLocalChecked(), GetKind, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState, nullptr);
  Nan::SetPrototypeMethod(tpl, "clone", Clone);
  Nan::SetPrototypeMethod(tpl, "stop", JsStop);
  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("MediaStreamTrack").ToLocalChecked(), tpl->GetFunction());
}
