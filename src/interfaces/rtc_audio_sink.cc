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

#include <v8.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/dictionaries/node_webrtc/rtc_on_data_event_dict.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"
#include "src/interfaces/media_stream_track.h"  // IWYU pragma: keep
#include "src/node/events.h"

namespace node_webrtc {

Nan::Persistent<v8::FunctionTemplate>& RTCAudioSink::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

RTCAudioSink::RTCAudioSink(rtc::scoped_refptr<webrtc::AudioTrackInterface> track)
  : AsyncObjectWrapWithLoop<RTCAudioSink>("RTCAudioSink", *this)
  , _track(std::move(track)) {
  _track->AddSink(this);
}

NAN_METHOD(RTCAudioSink::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCAudioSink.");
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(track, rtc::scoped_refptr<webrtc::AudioTrackInterface>);
  auto sink = new RTCAudioSink(track);
  sink->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCAudioSink::GetStopped) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<RTCAudioSink>::Unwrap(info.Holder());
  info.GetReturnValue().Set(self->_stopped);
}

void RTCAudioSink::Stop() {
  if (_track) {
    _stopped = true;
    _track->RemoveSink(this);
    _track = nullptr;
  }
  AsyncObjectWrapWithLoop<RTCAudioSink>::Stop();
}

NAN_METHOD(RTCAudioSink::JsStop) {
  auto self = AsyncObjectWrapWithLoop<RTCAudioSink>::Unwrap(info.Holder());
  self->Stop();
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

    Nan::HandleScope scope;
    auto maybeValue = From<v8::Local<v8::Value>>(dict);
    if (maybeValue.IsInvalid()) {
      // TODO(mroberts): Should raise an error; although this really shouldn't happen.
      // HACK(mroberts): I'd rather we use a smart pointer.
      delete[] dict.samples;
      return;
    }
    auto value = maybeValue.UnsafeFromValid();
    v8::Local<v8::Value> argv[1];
    argv[0] = value;
    MakeCallback("ondata", 1, argv);
  }));
}

void RTCAudioSink::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  RTCAudioSink::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCAudioSink").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("stopped").ToLocalChecked(), GetStopped, nullptr);
  Nan::SetPrototypeMethod(tpl, "stop", JsStop);
  exports->Set(Nan::New("RTCAudioSink").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace node_webrtc
