/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/getusermedia.h"

#include <webrtc/api/audio_options.h>
#include <webrtc/rtc_base/helpers.h>  // IWYU pragma: keep

#include "src/converters.h"  // IWYU pragma: keep
#include "src/converters/arguments.h"  // IWYU pragma: keep
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/converters/interfaces.h"  // IWYU pragma: keep
#include "src/converters/object.h"  // IWYU pragma: keep
#include "src/converters/v8.h"  // IWYU pragma: keep
#include "src/error.h"
#include "src/events.h"  // IWYU pragma: keep
#include "src/functional/curry.h"  // IWYU pragma: keep
#include "src/functional/either.h"  // IWYU pragma: keep
#include "src/functional/maybe.h"
#include "src/functional/operators.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/mediastream.h"  // IWYU pragma: keep
#include "src/peerconnectionfactory.h"  // IWYU pragma: keep
#include "src/rtcvideosource.h"  // IWYU pragma: keep

// TODO(mroberts): Expand support for other members.
struct MediaTrackConstraintSet {
  node_webrtc::Maybe<uint16_t> width;
  node_webrtc::Maybe<uint16_t> height;

  static MediaTrackConstraintSet Create(
      const node_webrtc::Maybe<uint16_t> width,
      const node_webrtc::Maybe<uint16_t> height
  ) {
    return {width, height};
  }
};

template <>
struct node_webrtc::Converter<v8::Local<v8::Value>, MediaTrackConstraintSet> {
  static node_webrtc::Validation<MediaTrackConstraintSet> Convert(const v8::Local<v8::Value> value) {
    return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<MediaTrackConstraintSet>(
    [](const v8::Local<v8::Object> object) {
      return curry(MediaTrackConstraintSet::Create)
          % node_webrtc::GetOptional<uint16_t>(object, "width")
          * node_webrtc::GetOptional<uint16_t>(object, "height");
    });
  }
};

struct MediaTrackConstraints: public MediaTrackConstraintSet {
  std::vector<MediaTrackConstraintSet> advanced;

  static MediaTrackConstraints Create(
      const MediaTrackConstraintSet set,
      const std::vector<MediaTrackConstraintSet>& advanced
  ) {
    MediaTrackConstraints constraints;
    constraints.width = set.width;
    constraints.height = set.height;
    constraints.advanced = advanced;
    return constraints;
  }
};

template <>
struct node_webrtc::Converter<v8::Local<v8::Value>, MediaTrackConstraints> {
  static node_webrtc::Validation<MediaTrackConstraints> Convert(const v8::Local<v8::Value> value) {
    return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<MediaTrackConstraints>(
    [&value](const v8::Local<v8::Object> object) {
      return curry(MediaTrackConstraints::Create)
          % node_webrtc::From<MediaTrackConstraintSet>(value)
          * node_webrtc::GetOptional<std::vector<MediaTrackConstraintSet>>(object, "advanced", std::vector<MediaTrackConstraintSet>());
    });
  }
};

struct MediaStreamConstraints {
  node_webrtc::Maybe<node_webrtc::Either<bool, MediaTrackConstraints>> audio;
  node_webrtc::Maybe<node_webrtc::Either<bool, MediaTrackConstraints>> video;

  static node_webrtc::Validation<MediaStreamConstraints> Create(
      const node_webrtc::Maybe<node_webrtc::Either<bool, MediaTrackConstraints>> audio,
      const node_webrtc::Maybe<node_webrtc::Either<bool, MediaTrackConstraints>> video
  ) {
    return audio.IsNothing() && video.IsNothing()
        ? node_webrtc::Validation<MediaStreamConstraints>::Invalid(R"(Must specify at least "audio" or "video")")
        : node_webrtc::Validation<MediaStreamConstraints>::Valid({audio, video});
  }
};

template <>
struct node_webrtc::Converter<v8::Local<v8::Value>, MediaStreamConstraints> {
  static node_webrtc::Validation<MediaStreamConstraints> Convert(const v8::Local<v8::Value> value) {
    return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<MediaStreamConstraints>(
    [](const v8::Local<v8::Object> object) {
      return node_webrtc::Validation<MediaStreamConstraints>::Join(curry(MediaStreamConstraints::Create)
              % GetOptional<node_webrtc::Either<bool, MediaTrackConstraints>>(object, "audio")
              * GetOptional<node_webrtc::Either<bool, MediaTrackConstraints>>(object, "video"));
    });
  }
};

NAN_METHOD(node_webrtc::GetUserMedia::GetUserMediaImpl) {
  SETUP_PROMISE(node_webrtc::GetUserMedia, node_webrtc::MediaStream*);
  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, constraints, MediaStreamConstraints);

  auto factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
  auto stream = factory->factory()->CreateLocalMediaStream(rtc::CreateRandomUuid());

  auto audio = constraints.audio.Map([](const node_webrtc::Either<bool, MediaTrackConstraints> constraint) {
    return constraint.FromLeft(true);
  }).FromMaybe(false);

  auto video = constraints.video.Map([](const node_webrtc::Either<bool, MediaTrackConstraints> constraint) {
    return constraint.FromLeft(true);
  }).FromMaybe(false);

  if (audio) {
    cricket::AudioOptions options;
    auto source = factory->factory()->CreateAudioSource(options);
    auto track = factory->factory()->CreateAudioTrack(rtc::CreateRandomUuid(), source);
    stream->AddTrack(track);
  }

  if (video) {
    auto source = new rtc::RefCountedObject<node_webrtc::RTCVideoTrackSource>();
    auto track = factory->factory()->CreateVideoTrack(rtc::CreateRandomUuid(), source);
    stream->AddTrack(track);
  }

  resolver->Resolve(Nan::GetCurrentContext(), MediaStream::wrap()->GetOrCreate(factory, stream)->handle()).IsNothing();
}

void node_webrtc::GetUserMedia::Init(v8::Handle<v8::Object> exports) {
  Nan::SetMethod(exports, "getUserMedia", GetUserMediaImpl);
}
