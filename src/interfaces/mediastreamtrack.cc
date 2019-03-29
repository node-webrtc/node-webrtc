/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/mediastreamtrack.h"

#include "src/converters.h"
#include "src/converters/dictionaries.h"
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/peerconnectionfactory.h"

Nan::Persistent<v8::Function>& node_webrtc::MediaStreamTrack::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& node_webrtc::MediaStreamTrack::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

node_webrtc::MediaStreamTrack::MediaStreamTrack(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>&& track)
  : node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>("MediaStreamTrack", *this)
  , _factory(std::move(factory))
  , _track(std::move(track)) {
  _track->RegisterObserver(this);
}

node_webrtc::MediaStreamTrack::~MediaStreamTrack() {
  _track->UnregisterObserver(this);
  wrap()->Release(this);
}

NAN_METHOD(node_webrtc::MediaStreamTrack::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a MediaStreamTrack");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto track = *static_cast<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new node_webrtc::MediaStreamTrack(std::move(factory), std::move(track));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

void node_webrtc::MediaStreamTrack::Stop() {
  _ended = true;
  _enabled = _track->enabled();
  node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Stop();
}

void node_webrtc::MediaStreamTrack::OnChanged() {
  if (this->_track->state() == webrtc::MediaStreamTrackInterface::TrackState::kEnded) {
    Stop();
  }
}

void node_webrtc::MediaStreamTrack::OnPeerConnectionClosed() {
  Stop();
}

NAN_GETTER(node_webrtc::MediaStreamTrack::GetEnabled) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_ended ? self->_enabled : self->_track->enabled()));
}

NAN_SETTER(node_webrtc::MediaStreamTrack::SetEnabled) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, enabled, bool);

  if (self->_ended) {
    self->_enabled = enabled;
  } else {
    self->_track->set_enabled(enabled);
  }
}

NAN_GETTER(node_webrtc::MediaStreamTrack::GetId) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->id()).ToLocalChecked());
}

NAN_GETTER(node_webrtc::MediaStreamTrack::GetKind) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_track->kind()).ToLocalChecked());
}

NAN_GETTER(node_webrtc::MediaStreamTrack::GetReadyState) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(info.Holder());
  auto state = self->_ended
      ? webrtc::MediaStreamTrackInterface::TrackState::kEnded
      : self->_track->state();
  CONVERT_OR_THROW_AND_RETURN(state, result, std::string);
  info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
}

NAN_GETTER(node_webrtc::MediaStreamTrack::GetMuted) {
  (void) property;
  info.GetReturnValue().Set(false);
}

NAN_METHOD(node_webrtc::MediaStreamTrack::Clone) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(info.Holder());
  auto label = rtc::CreateRandomUuid();
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> clonedTrack = nullptr;
  if (self->_track->kind() == self->_track->kAudioKind) {
    auto audioTrack = static_cast<webrtc::AudioTrackInterface*>(self->_track.get());
    clonedTrack = self->_factory->factory()->CreateAudioTrack(label, audioTrack->GetSource());
  } else {
    auto videoTrack = static_cast<webrtc::VideoTrackInterface*>(self->_track.get());
    clonedTrack = self->_factory->factory()->CreateVideoTrack(label, videoTrack->GetSource());
  }
  auto clonedMediaStreamTrack = wrap()->GetOrCreate(self->_factory, clonedTrack);
  if (self->_ended) {
    clonedMediaStreamTrack->Stop();
  }
  info.GetReturnValue().Set(clonedMediaStreamTrack->ToObject());
}

NAN_METHOD(node_webrtc::MediaStreamTrack::JsStop) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(info.Holder());
  self->Stop();
}

node_webrtc::Wrap <
node_webrtc::MediaStreamTrack*,
rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>,
std::shared_ptr<node_webrtc::PeerConnectionFactory>
> * node_webrtc::MediaStreamTrack::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  node_webrtc::MediaStreamTrack*,
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>,
  std::shared_ptr<node_webrtc::PeerConnectionFactory>
  > (node_webrtc::MediaStreamTrack::Create);
  return wrap;
}

node_webrtc::MediaStreamTrack* node_webrtc::MediaStreamTrack::Create(
    std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&track));
  auto mediaStreamTrack = Nan::NewInstance(Nan::New(node_webrtc::MediaStreamTrack::constructor()), 2, cargv).ToLocalChecked();
  return node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap(mediaStreamTrack);
}

void node_webrtc::MediaStreamTrack::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  node_webrtc::MediaStreamTrack::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("MediaStreamTrack").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("enabled").ToLocalChecked(), GetEnabled, SetEnabled);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("kind").ToLocalChecked(), GetKind, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("readyState").ToLocalChecked(), GetReadyState, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("muted").ToLocalChecked(), GetMuted, nullptr);
  Nan::SetPrototypeMethod(tpl, "clone", Clone);
  Nan::SetPrototypeMethod(tpl, "stop", JsStop);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("MediaStreamTrack").ToLocalChecked(), tpl->GetFunction());
}
