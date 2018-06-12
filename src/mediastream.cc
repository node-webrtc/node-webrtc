/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/mediastream.h"

#include "webrtc/base/helpers.h"

#include "src/converters/arguments.h"
#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/converters/webrtc.h"

using node_webrtc::BidiMap;
using node_webrtc::Either;
using node_webrtc::Maybe;
using node_webrtc::MediaStream;
using node_webrtc::MediaStreamTrack;
using node_webrtc::PeerConnectionFactory;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> MediaStream::constructor;

BidiMap<rtc::scoped_refptr<webrtc::MediaStreamInterface>, MediaStream*> MediaStream::_streams;

MediaStream::MediaStream(std::shared_ptr<PeerConnectionFactory>&& factory)
  : _factory(factory ? factory : PeerConnectionFactory::GetOrCreateDefault())
  , _stream(_factory->factory()->CreateLocalMediaStream(rtc::CreateRandomUuid()))
  , _shouldReleaseFactory(!factory) {
}

MediaStream::MediaStream(std::vector<MediaStreamTrack*>&& tracks, std::shared_ptr<PeerConnectionFactory>&& factory)
  : _factory(factory ? factory : tracks.empty() ? PeerConnectionFactory::GetOrCreateDefault() : tracks[0]->factory())
  , _stream(_factory->factory()->CreateLocalMediaStream(rtc::CreateRandomUuid()))
  , _shouldReleaseFactory(!factory && tracks.empty()) {
  for (auto const& track : tracks) {
    if (track->track()->kind() == track->track()->kAudioKind) {
      auto audioTrack = static_cast<webrtc::AudioTrackInterface*>(track->track().get());
      _stream->AddTrack(audioTrack);
    } else {
      auto videoTrack = static_cast<webrtc::VideoTrackInterface*>(track->track().get());
      _stream->AddTrack(videoTrack);
    }
  }
}

MediaStream::MediaStream(
    rtc::scoped_refptr<webrtc::MediaStreamInterface>&& stream,
    std::shared_ptr<PeerConnectionFactory>&& factory)
  : _factory(factory ? factory : PeerConnectionFactory::GetOrCreateDefault())
  , _stream(stream)
  , _shouldReleaseFactory(!factory) {
}

MediaStream::~MediaStream() {
  MediaStream::Release(this);
  if (_shouldReleaseFactory) {
    PeerConnectionFactory::Release();
  }
}

NAN_METHOD(MediaStream::New) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(eithers, Either<std::tuple<Local<External> COMMA Local<External>> COMMA Either<std::vector<MediaStreamTrack*> COMMA Maybe<MediaStream*>>>);

  MediaStream* mediaStream = nullptr;

  if (eithers.IsLeft()) {
    // 1. Remote MediaStream
    auto pair = eithers.UnsafeFromLeft();
    auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(Local<External>::Cast(std::get<0>(pair))->Value());
    auto stream = *static_cast<rtc::scoped_refptr<webrtc::MediaStreamInterface>*>(Local<External>::Cast(std::get<1>(pair))->Value());
    mediaStream = new MediaStream(std::move(stream), std::move(factory));
  } else {
    auto either = eithers.UnsafeFromRight();
    if (either.IsLeft()) {
      // 2. Local MediaStream, Array of MediaStreamTracks
      auto tracks = either.UnsafeFromLeft();
      mediaStream = new MediaStream(std::move(tracks));
    } else {
      auto maybeStream = either.UnsafeFromRight();
      if (maybeStream.IsJust()) {
        // 3. Local MediaStream, existing MediaStream
        auto existingStream = maybeStream.UnsafeFromJust();
        auto factory = existingStream->_factory;
        auto tracks = std::vector<MediaStreamTrack*>();
        for (auto const& track : existingStream->tracks()) {
          tracks.push_back(MediaStreamTrack::GetOrCreate(factory, track));
        }
        mediaStream = new MediaStream(std::move(tracks), std::move(factory));
      } else {
        // 4. Local MediaStream
        mediaStream = new MediaStream();
      }
    }
  }

  mediaStream->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(MediaStream::GetId) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  info.GetReturnValue().Set(Nan::New(self->_stream->label()).ToLocalChecked());
}

NAN_GETTER(MediaStream::GetActive) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto active = false;
  for (auto const& track : self->tracks()) {
    active = active || track->state() == webrtc::MediaStreamTrackInterface::TrackState::kLive;
  }
  info.GetReturnValue().Set(Nan::New(active));
}

NAN_METHOD(MediaStream::GetAudioTracks) {
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto tracks = std::vector<MediaStreamTrack*>();
  for (auto const& track : self->_stream->GetAudioTracks()) {
    auto mediaStreamTrack = MediaStreamTrack::GetOrCreate(self->_factory, track);
    tracks.push_back(mediaStreamTrack);
  }
  CONVERT_OR_THROW_AND_RETURN(tracks, result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(MediaStream::GetVideoTracks) {
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto tracks = std::vector<MediaStreamTrack*>();
  for (auto const& track : self->_stream->GetVideoTracks()) {
    auto mediaStreamTrack = MediaStreamTrack::GetOrCreate(self->_factory, track);
    tracks.push_back(mediaStreamTrack);
  }
  CONVERT_OR_THROW_AND_RETURN(tracks, result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(MediaStream::GetTracks) {
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto tracks = std::vector<MediaStreamTrack*>();
  for (auto const& track : self->tracks()) {
    auto mediaStreamTrack = MediaStreamTrack::GetOrCreate(self->_factory, track);
    tracks.push_back(mediaStreamTrack);
  }
  CONVERT_OR_THROW_AND_RETURN(tracks, result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(MediaStream::GetTrackById) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(label, std::string);
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto audioTrack = self->_stream->FindAudioTrack(label);
  if (audioTrack) {
    auto track = MediaStreamTrack::GetOrCreate(self->_factory, audioTrack);
    info.GetReturnValue().Set(track->ToObject());
  }
  auto videoTrack = self->_stream->FindAudioTrack(label);
  if (videoTrack) {
    auto track = MediaStreamTrack::GetOrCreate(self->_factory, videoTrack);
    info.GetReturnValue().Set(track->ToObject());
  }
}

NAN_METHOD(MediaStream::AddTrack) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(mediaStreamTrack, MediaStreamTrack*);
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto stream = self->_stream;
  auto track = mediaStreamTrack->track();
  if (track->kind() == track->kAudioKind) {
    stream->AddTrack(static_cast<webrtc::AudioTrackInterface*>(track.get()));
  } else {
    stream->AddTrack(static_cast<webrtc::VideoTrackInterface*>(track.get()));
  }
}

NAN_METHOD(MediaStream::RemoveTrack) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(mediaStreamTrack, MediaStreamTrack*);
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto stream = self->_stream;
  auto track = mediaStreamTrack->track();
  if (track->kind() == track->kAudioKind) {
    stream->RemoveTrack(static_cast<webrtc::AudioTrackInterface*>(track.get()));
  } else {
    stream->RemoveTrack(static_cast<webrtc::VideoTrackInterface*>(track.get()));
  }
}

NAN_METHOD(MediaStream::Clone) {
  (void) info;
  auto self = Nan::ObjectWrap::Unwrap<MediaStream>(info.Holder());
  auto clonedMediaStreamTracks = std::vector<Local<Value>>();
  for (auto const& track : self->tracks()) {
    auto mediaStreamTrack = MediaStreamTrack::GetOrCreate(self->_factory, track);
    auto clonedMediaStreamTrack = Nan::Call("clone", mediaStreamTrack->ToObject(), 0, nullptr);
    if (!clonedMediaStreamTrack.IsEmpty()) {
      clonedMediaStreamTracks.push_back(clonedMediaStreamTrack.ToLocalChecked());
    }
  }
  CONVERT_OR_THROW_AND_RETURN(clonedMediaStreamTracks, tracks, Local<Value>);
  Local<Value> cargv[1];
  cargv[0] = tracks;
  auto mediaStream = Nan::NewInstance(Nan::New(MediaStream::constructor), 1, cargv).ToLocalChecked();
  info.GetReturnValue().Set(mediaStream);
}

MediaStream* MediaStream::GetOrCreate(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  return _streams.computeIfAbsent(stream, [&factory, &stream]() {
    Nan::HandleScope scope;
    Local<Value> cargv[2];
    cargv[0] = Nan::New<External>(static_cast<void*>(&factory));
    cargv[1] = Nan::New<External>(static_cast<void*>(&stream));
    auto mediaStream = Nan::NewInstance(Nan::New(MediaStream::constructor), 2, cargv).ToLocalChecked();
    return Nan::ObjectWrap::Unwrap<MediaStream>(mediaStream);
  });
}

void MediaStream::Release(MediaStream* stream) {
  _streams.reverseRemove(stream);
}

void MediaStream::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("MediaStream").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("id").ToLocalChecked(), GetId, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("active").ToLocalChecked(), GetActive, nullptr);
  Nan::SetPrototypeMethod(tpl, "getAudioTracks", GetAudioTracks);
  Nan::SetPrototypeMethod(tpl, "getVideoTracks", GetVideoTracks);
  Nan::SetPrototypeMethod(tpl, "getTracks", GetTracks);
  Nan::SetPrototypeMethod(tpl, "getTrackById", GetTrackById);
  Nan::SetPrototypeMethod(tpl, "addTrack", AddTrack);
  Nan::SetPrototypeMethod(tpl, "removeTrack", RemoveTrack);
  Nan::SetPrototypeMethod(tpl, "clone", Clone);
  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("MediaStream").ToLocalChecked(), tpl->GetFunction());
}
