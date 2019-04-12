/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_audio_sink.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <type_traits>
#include <utility>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/napi.h"
#include "src/dictionaries/node_webrtc/rtc_on_data_event_dict.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"
#include "src/interfaces/media_stream_track.h"  // IWYU pragma: keep
#include "src/node/events.h"

namespace node_webrtc {

Napi::FunctionReference& RTCAudioSink::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCAudioSink::RTCAudioSink(const Napi::CallbackInfo& info)
  : AsyncObjectWrapWithLoop<RTCAudioSink>("RTCAudioSink", *this, info) {
  auto env = info.Env();

  if (!info.IsConstructCall()) {
    Napi::TypeError::New(env, "Use the new operator to construct an RTCAudioSink.");
    return;
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN_VOID_NAPI(info, track, rtc::scoped_refptr<webrtc::AudioTrackInterface>)

  _track = std::move(track);
  _track->AddSink(this);
}

Napi::Value RTCAudioSink::GetStopped(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _stopped, result, Napi::Value)
  return result;
}

void RTCAudioSink::Stop() {
  if (_track) {
    _stopped = true;
    _track->RemoveSink(this);
    _track = nullptr;
  }
  AsyncObjectWrapWithLoop<RTCAudioSink>::Stop();
}

Napi::Value RTCAudioSink::JsStop(const Napi::CallbackInfo& info) {
  Stop();
  return info.Env().Undefined();
}

void RTCAudioSink::OnData(
    const void* audio_data,
    int bits_per_sample,
    int sample_rate,
    size_t number_of_channels,
    size_t number_of_frames) {
  auto byte_length = number_of_channels * number_of_frames * bits_per_sample / 8;
  std::unique_ptr<uint8_t[]> audio_data_copy(new uint8_t[byte_length]);
  if (!audio_data_copy) {
    // TODO(mroberts): Throw an error somehow?
    return;
  }
  memcpy(audio_data_copy.get(), audio_data, byte_length);

  Dispatch(CreateCallback<RTCAudioSink>([
             this,
             audio_data_copy = std::move(audio_data_copy),
             bits_per_sample,
             sample_rate,
             number_of_channels,
             number_of_frames
  ]() mutable {
    RTCOnDataEventDict dict({
      audio_data_copy.release(),
      static_cast<uint8_t>(bits_per_sample),
      static_cast<uint16_t>(sample_rate),
      static_cast<uint8_t>(number_of_channels),
      MakeJust<uint16_t>(static_cast<uint16_t>(number_of_frames))
    });

    auto env = Env();
    Napi::HandleScope scope(env);
    auto maybeValue = From<Napi::Value>(std::make_pair(env, dict));
    if (maybeValue.IsInvalid()) {
      // TODO(mroberts): Should raise an error; although this really shouldn't happen.
      // HACK(mroberts): I'd rather we use a smart pointer.
      delete[] dict.samples;
      return;
    }
    MakeCallback("ondata", { maybeValue.UnsafeFromValid() });
  }));
}

void RTCAudioSink::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "RTCAudioSink", {
    InstanceAccessor("stopped", &RTCAudioSink::GetStopped, nullptr),
    InstanceMethod("stop", &RTCAudioSink::JsStop)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCAudioSink", func);
}

}  // namespace node_webrtc
