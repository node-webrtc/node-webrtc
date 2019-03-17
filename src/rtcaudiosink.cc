/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcaudiosink.h"

#include "src/converters.h"  // IWYU pragma: keep
#include "src/converters/arguments.h"  // IWYU pragma: keep
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/converters/interfaces.h"  // IWYU pragma: keep
#include "src/error.h"
#include "src/events.h"  // IWYU pragma: keep

// IWYU pragma: no_include <api/mediastreaminterface.h>
// IWYU pragma: no_include <rtc_base/scoped_ref_ptr.h>
// IWYU pragma: no_forward_declare webrtc::AudioTrackInterface

Nan::Persistent<v8::FunctionTemplate>& node_webrtc::RTCAudioSink::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

node_webrtc::RTCAudioSink::RTCAudioSink(rtc::scoped_refptr<webrtc::AudioTrackInterface> track)
  : node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCAudioSink>("RTCAudioSink", *this)
  , _track(std::move(track)) {
  _track->AddSink(this);
}

NAN_METHOD(node_webrtc::RTCAudioSink::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCAudioSink.");
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(track, rtc::scoped_refptr<webrtc::AudioTrackInterface>);
  auto sink = new RTCAudioSink(track);
  sink->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(node_webrtc::RTCAudioSink::GetStopped) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<node_webrtc::RTCAudioSink>::Unwrap(info.Holder());
  info.GetReturnValue().Set(self->_stopped);
}

void node_webrtc::RTCAudioSink::Stop() {
  if (_track) {
    _stopped = true;
    _track->RemoveSink(this);
    _track = nullptr;
  }
  node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCAudioSink>::Stop();
}

NAN_METHOD(node_webrtc::RTCAudioSink::JsStop) {
  auto self = AsyncObjectWrapWithLoop<node_webrtc::RTCAudioSink>::Unwrap(info.Holder());
  self->Stop();
}

void node_webrtc::RTCAudioSink::OnData(
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

  Dispatch(node_webrtc::OnDataEvent::Create({
    audio_data_copy.release(),
    static_cast<uint8_t>(bits_per_sample),
    static_cast<uint16_t>(sample_rate),
    static_cast<uint8_t>(number_of_channels),
    node_webrtc::MakeJust<uint16_t>(static_cast<uint16_t>(number_of_frames))
  }));
}

void node_webrtc::RTCAudioSink::HandleOnDataEvent(const node_webrtc::OnDataEvent& event) {
  Nan::HandleScope scope;
  auto maybeValue = node_webrtc::From<v8::Local<v8::Value>>(event.dict);
  if (maybeValue.IsInvalid()) {
    // TODO(mroberts): Should raise an error; although this really shouldn't happen.
    // HACK(mroberts): I'd rather we use a smart pointer.
    delete[] event.dict.samples;
    return;
  }
  auto value = maybeValue.UnsafeFromValid();
  v8::Local<v8::Value> argv[1];
  argv[0] = value;
  MakeCallback("ondata", 1, argv);
}

void node_webrtc::RTCAudioSink::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  node_webrtc::RTCAudioSink::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCAudioSink").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("stopped").ToLocalChecked(), GetStopped, nullptr);
  Nan::SetPrototypeMethod(tpl, "stop", JsStop);
  exports->Set(Nan::New("RTCAudioSink").ToLocalChecked(), tpl->GetFunction());
}
