/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/media_stream_track.h"

#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/rtc_base/helpers.h>

#include "src/converters.h"
#include "src/converters/interfaces.h"
#include "src/converters/v8.h"
#include "src/enums/webrtc/track_state.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"
#include "src/node/error.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& MediaStreamTrack::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& MediaStreamTrack::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

MediaStreamTrack::MediaStreamTrack(
    std::shared_ptr<PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>&& track)
  : AsyncObjectWrapWithLoop<MediaStreamTrack>("MediaStreamTrack", *this)
  , _factory(std::move(factory))
  , _track(std::move(track)) {
  _track->RegisterObserver(this);
}

MediaStreamTrack::~MediaStreamTrack() {
  _track->UnregisterObserver(this);
  wrap()->Release(this);
}

NAN_METHOD(MediaStreamTrack::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a MediaStreamTrack");
  }

  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto track = *static_cast<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new MediaStreamTrack(std::move(factory), std::move(track));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

void MediaStreamTrack::Stop() {
  _ended = true;
  _enabled = _track->enabled();
  AsyncObjectWrapWithLoop<MediaStreamTrack>::Stop();
}

void MediaStreamTrack::OnChanged() {
  if (this->_track->state() == webrtc::MediaStreamTrackInterface::TrackState::kEnded) {
    Stop();
  }
}

void MediaStreamTrack::OnPeerConnectionClosed() {
  Stop();
}

NAN_GETTER(MediaStreamTrack::GetEnabled) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_ended ? self->_enabled : self->_track->enabled()));
}

NAN_SETTER(MediaStreamTrack::SetEnabled) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, enabled, bool)

  if (self->_ended) {
    self->_enabled = enabled;
  } else {
    self->_track->set_enabled(enabled);
  }
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
  auto state = self->_ended
      ? webrtc::MediaStreamTrackInterface::TrackState::kEnded
      : self->_track->state();
  CONVERT_OR_THROW_AND_RETURN(state, result, std::string)
  info.GetReturnValue().Set(Nan::New(result).ToLocalChecked());
}

NAN_GETTER(MediaStreamTrack::GetMuted) {
  (void) property;
  info.GetReturnValue().Set(false);
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
  auto clonedMediaStreamTrack = wrap()->GetOrCreate(self->_factory, clonedTrack);
  if (self->_ended) {
    clonedMediaStreamTrack->Stop();
  }
  info.GetReturnValue().Set(clonedMediaStreamTrack->ToObject());
}

NAN_METHOD(MediaStreamTrack::JsStop) {
  auto self = AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(info.Holder());
  self->Stop();
}

Wrap <
MediaStreamTrack*,
rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>,
std::shared_ptr<PeerConnectionFactory>
> * MediaStreamTrack::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  MediaStreamTrack*,
  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > (MediaStreamTrack::Create);
  return wrap;
}

MediaStreamTrack* MediaStreamTrack::Create(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&track));
  auto mediaStreamTrack = Nan::NewInstance(Nan::New(MediaStreamTrack::constructor()), 2, cargv).ToLocalChecked();
  return AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap(mediaStreamTrack);
}

void MediaStreamTrack::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  MediaStreamTrack::tpl().Reset(tpl);
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

CONVERT_INTERFACE_TO_AND_FROM_JS(MediaStreamTrack, "MediaStreamTrack", ToObject, AsyncObjectWrapWithLoop<MediaStreamTrack>::Unwrap)

CONVERTER_IMPL(MediaStreamTrack*, rtc::scoped_refptr<webrtc::AudioTrackInterface>, mediaStreamTrack) {
  auto track = mediaStreamTrack->track();
  if (track->kind() != webrtc::MediaStreamTrackInterface::kAudioKind) {
    return Validation<rtc::scoped_refptr<webrtc::AudioTrackInterface>>::Invalid(
            "Expected an audio MediaStreamTrack");
  }
  rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack(static_cast<webrtc::AudioTrackInterface*>(track.get()));
  return Pure(audioTrack);
}

CONVERTER_IMPL(MediaStreamTrack*, rtc::scoped_refptr<webrtc::VideoTrackInterface>, mediaStreamTrack) {
  auto track = mediaStreamTrack->track();
  if (track->kind() != webrtc::MediaStreamTrackInterface::kVideoKind) {
    return Validation<rtc::scoped_refptr<webrtc::VideoTrackInterface>>::Invalid(
            "Expected a video MediaStreamTrack");
  }
  rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack(static_cast<webrtc::VideoTrackInterface*>(track.get()));
  return Pure(videoTrack);
}

CONVERT_VIA(v8::Local<v8::Value>, MediaStreamTrack*, rtc::scoped_refptr<webrtc::AudioTrackInterface>)
CONVERT_VIA(v8::Local<v8::Value>, MediaStreamTrack*, rtc::scoped_refptr<webrtc::VideoTrackInterface>)

}  // namespace node_webrtc
