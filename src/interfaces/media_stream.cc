/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/media_stream.h"

#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/interfaces.h"
#include "src/converters/napi.h"
#include "src/functional/either.h"
#include "src/functional/maybe.h"
#include "src/dictionaries/node_webrtc/rtc_media_stream_init.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"

namespace node_webrtc {

Napi::FunctionReference& MediaStream::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

MediaStream::Impl::Impl(PeerConnectionFactory* factory)
  : _factory(factory ? factory : PeerConnectionFactory::GetOrCreateDefault())
  , _stream(_factory->factory()->CreateLocalMediaStream(rtc::CreateRandomUuid()))
  , _shouldReleaseFactory(!factory) {
  _factory->Ref();
}

MediaStream::Impl::Impl(std::vector<MediaStreamTrack*>&& tracks, PeerConnectionFactory* factory)
  : _factory(factory ? factory : tracks.empty() ? PeerConnectionFactory::GetOrCreateDefault() : tracks[0]->factory())
  , _stream(_factory->factory()->CreateLocalMediaStream(rtc::CreateRandomUuid()))
  , _shouldReleaseFactory(!factory && tracks.empty()) {
  _factory->Ref();
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

MediaStream::Impl::Impl(
    rtc::scoped_refptr<webrtc::MediaStreamInterface>&& stream,
    PeerConnectionFactory* factory)
  : _factory(factory ? factory : PeerConnectionFactory::GetOrCreateDefault())
  , _stream(stream)
  , _shouldReleaseFactory(!factory) {
  _factory->Ref();
}

MediaStream::Impl::Impl(const RTCMediaStreamInit& init,
    PeerConnectionFactory* factory)
  : _factory(factory ? factory : PeerConnectionFactory::GetOrCreateDefault())
  , _stream(_factory->factory()->CreateLocalMediaStream(init.id))
  , _shouldReleaseFactory(!factory) {
  _factory->Ref();
}

MediaStream::Impl::~Impl() {
  Napi::HandleScope scope(PeerConnectionFactory::constructor().Env());
  if (_factory) {
    _factory->Unref();  // NOLINT
    _factory = nullptr;
  }
  if (_shouldReleaseFactory) {
    PeerConnectionFactory::Release();
  }
}

std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>> MediaStream::tracks() {
  auto tracks = std::vector<rtc::scoped_refptr<webrtc::MediaStreamTrackInterface>>();
  for (auto const& track : _impl._stream->GetAudioTracks()) {
    tracks.emplace_back(track);
  }
  for (auto const& track : _impl._stream->GetVideoTracks()) {
    tracks.emplace_back(track);
  }
  return tracks;
}

rtc::scoped_refptr<webrtc::MediaStreamInterface> MediaStream::stream() {
  return _impl._stream;
}

MediaStream::MediaStream(const Napi::CallbackInfo& info): Napi::ObjectWrap<MediaStream>(info) {
  auto maybeEither = From<Either<std::tuple<Napi::Object COMMA Napi::External<rtc::scoped_refptr<webrtc::MediaStreamInterface>>> COMMA   // Either1 - Remote MediaStream OR Either2
      Either<std::vector<MediaStreamTrack*> COMMA                                                                  // Either2 - Array of MediaStreamTracks OR Either3
      Either<MediaStream* COMMA                                                                                  // Either3 - Local MediaStream OR Maybe
      Maybe<RTCMediaStreamInit>>>>>(Arguments(info));                                                          // Maybe - Optional RTCMediaStreamInit dictionary
  if (maybeEither.IsInvalid()) {
    Napi::TypeError::New(info.Env(), maybeEither.ToErrors()[0]).ThrowAsJavaScriptException();
    return;
  }
  auto either1 = maybeEither.UnsafeFromValid();

  if (either1.IsLeft()) {
    // 1. Remote MediaStream
    auto pair = either1.UnsafeFromLeft();
    // FIXME(mroberts): There is a safer way to do this.
    auto factory = PeerConnectionFactory::Unwrap(std::get<0>(pair));
    auto stream = *std::get<1>(pair).Data();
    _impl = MediaStream::Impl(std::move(stream), factory);
  } else {
    auto either2 = either1.UnsafeFromRight();
    if (either2.IsLeft()) {
      // 2. Local MediaStream, Array of MediaStreamTracks
      auto tracks = either2.UnsafeFromLeft();
      _impl = MediaStream::Impl(std::move(tracks));
    } else {
      auto either3 = either2.UnsafeFromRight();
      if (either3.IsLeft()) {
        // 3. Local MediaStream, existing MediaStream
        auto existingStream = either3.UnsafeFromLeft();
        auto factory = existingStream->_impl._factory;
        auto tracks = std::vector<MediaStreamTrack*>();
        for (auto const& track : existingStream->tracks()) {
          tracks.push_back(MediaStreamTrack::wrap()->GetOrCreate(factory, track));
        }
        _impl = MediaStream::Impl(std::move(tracks), factory);
      } else {
        // Check if RTCMediaStreamInit was provided
        auto maybeMediaStreamInit = either3.UnsafeFromRight();
        if (maybeMediaStreamInit.IsJust()) {
          // 4. Local MediaStream with Custom MediaStreamId
          _impl = MediaStream::Impl(maybeMediaStreamInit.UnsafeFromJust());
        } else {
          // 5. Local MediaStream
          _impl = MediaStream::Impl();
        }
      }
    }
  }
}

Napi::Value MediaStream::GetId(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _impl._stream->id(), result, Napi::Value)
  return result;
}

Napi::Value MediaStream::GetActive(const Napi::CallbackInfo& info) {
  auto active = false;
  for (auto const& track : tracks()) {
    auto mediaStreamTrack = MediaStreamTrack::wrap()->GetOrCreate(_impl._factory, track);
    active = active || mediaStreamTrack->active();
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), active, result, Napi::Value)
  return result;
}

Napi::Value MediaStream::GetAudioTracks(const Napi::CallbackInfo& info) {
  auto tracks = std::vector<MediaStreamTrack*>();
  for (auto const& track : _impl._stream->GetAudioTracks()) {
    auto mediaStreamTrack = MediaStreamTrack::wrap()->GetOrCreate(_impl._factory, track);
    tracks.push_back(mediaStreamTrack);
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), tracks, result, Napi::Value)
  return result;
}

Napi::Value MediaStream::GetVideoTracks(const Napi::CallbackInfo& info) {
  auto tracks = std::vector<MediaStreamTrack*>();
  for (auto const& track : _impl._stream->GetVideoTracks()) {
    auto mediaStreamTrack = MediaStreamTrack::wrap()->GetOrCreate(_impl._factory, track);
    tracks.push_back(mediaStreamTrack);
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), tracks, result, Napi::Value)
  return result;
}

Napi::Value MediaStream::GetTracks(const Napi::CallbackInfo& info) {
  auto tracks = std::vector<MediaStreamTrack*>();
  for (auto const& track : this->tracks()) {
    auto mediaStreamTrack = MediaStreamTrack::wrap()->GetOrCreate(_impl._factory, track);
    tracks.push_back(mediaStreamTrack);
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), tracks, result, Napi::Value)
  return result;
}

Napi::Value MediaStream::GetTrackById(const Napi::CallbackInfo& info) {
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, label, std::string)
  auto audioTrack = _impl._stream->FindAudioTrack(label);
  if (audioTrack) {
    auto track = MediaStreamTrack::wrap()->GetOrCreate(_impl._factory, audioTrack);
    CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), track, result, Napi::Value)
    return result;
  }
  auto videoTrack = _impl._stream->FindVideoTrack(label);
  if (videoTrack) {
    auto track = MediaStreamTrack::wrap()->GetOrCreate(_impl._factory, videoTrack);
    CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), track, result, Napi::Value)
    return result;
  }
  return info.Env().Null();
}

Napi::Value MediaStream::AddTrack(const Napi::CallbackInfo& info) {
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, mediaStreamTrack, MediaStreamTrack*)
  auto stream = _impl._stream;
  auto track = mediaStreamTrack->track();
  if (track->kind() == track->kAudioKind) {
    stream->AddTrack(static_cast<webrtc::AudioTrackInterface*>(track.get()));
  } else {
    stream->AddTrack(static_cast<webrtc::VideoTrackInterface*>(track.get()));
  }
  return info.Env().Undefined();
}

Napi::Value MediaStream::RemoveTrack(const Napi::CallbackInfo& info) {
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, mediaStreamTrack, MediaStreamTrack*)
  auto stream = _impl._stream;
  auto track = mediaStreamTrack->track();
  if (track->kind() == track->kAudioKind) {
    stream->RemoveTrack(static_cast<webrtc::AudioTrackInterface*>(track.get()));
  } else {
    stream->RemoveTrack(static_cast<webrtc::VideoTrackInterface*>(track.get()));
  }
  return info.Env().Undefined();
}

Napi::Value MediaStream::Clone(const Napi::CallbackInfo& info) {
  auto clonedStream = _impl._factory->factory()->CreateLocalMediaStream(rtc::CreateRandomUuid());
  for (auto const& track : this->tracks()) {
    if (track->kind() == track->kAudioKind) {
      auto audioTrack = static_cast<webrtc::AudioTrackInterface*>(track.get());
      auto source = audioTrack->GetSource();
      auto clonedTrack = _impl._factory->factory()->CreateAudioTrack(rtc::CreateRandomUuid(), source);
      clonedStream->AddTrack(clonedTrack);
    } else {
      auto videoTrack = static_cast<webrtc::VideoTrackInterface*>(track.get());
      auto source = videoTrack->GetSource();
      auto clonedTrack = _impl._factory->factory()->CreateVideoTrack(rtc::CreateRandomUuid(), source);
      clonedStream->AddTrack(clonedTrack);
    }
  }
  auto mediaStream = MediaStream::wrap()->GetOrCreate(_impl._factory, clonedStream);
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), mediaStream, result, Napi::Value)
  return result;
}

Wrap <
MediaStream*,
rtc::scoped_refptr<webrtc::MediaStreamInterface>,
PeerConnectionFactory*
> * MediaStream::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  MediaStream*,
  rtc::scoped_refptr<webrtc::MediaStreamInterface>,
  PeerConnectionFactory*
  > (MediaStream::Create);
  return wrap;
}

MediaStream* MediaStream::Create(
    PeerConnectionFactory* factory,
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  auto env = MediaStream::constructor().Env();
  Napi::HandleScope scope(env);

  auto object = MediaStream::constructor().New({
    factory->Value(),
    Napi::External<rtc::scoped_refptr<webrtc::MediaStreamInterface>>::New(env, &stream)
  });

  return MediaStream::Unwrap(object);
}

void MediaStream::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "MediaStream", {
    InstanceAccessor("id", &MediaStream::GetId, nullptr),
    InstanceAccessor("active", &MediaStream::GetActive, nullptr),
    InstanceMethod("getAudioTracks", &MediaStream::GetAudioTracks),
    InstanceMethod("getVideoTracks", &MediaStream::GetVideoTracks),
    InstanceMethod("getTracks", &MediaStream::GetTracks),
    InstanceMethod("getTrackById", &MediaStream::GetTrackById),
    InstanceMethod("addTrack", &MediaStream::AddTrack),
    InstanceMethod("removeTrack", &MediaStream::RemoveTrack),
    InstanceMethod("clone", &MediaStream::Clone),
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("MediaStream", func);
}

FROM_NAPI_IMPL(MediaStream*, value) {
  return From<Napi::Object>(value).FlatMap<MediaStream*>([](Napi::Object object) {
    auto isMediaStream = false;
    napi_instanceof(object.Env(), object, MediaStream::constructor().Value(), &isMediaStream);
    if (object.Env().IsExceptionPending()) {
      return Validation<MediaStream*>::Invalid(object.Env().GetAndClearPendingException().Message());
    } else if (!isMediaStream) {
      return Validation<MediaStream*>::Invalid("This is not an instance of MediaStream");
    }
    return Pure(MediaStream::Unwrap(object));
  });
}

TO_NAPI_IMPL(MediaStream*, pair) {
  return Pure(pair.second->Value().As<Napi::Value>());
}

}  // namespace node_webrtc
