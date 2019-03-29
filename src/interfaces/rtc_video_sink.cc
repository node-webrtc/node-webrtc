/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_video_sink.h"

#include <type_traits>

#include <webrtc/api/video/video_source_interface.h>
#include <webrtc/api/video/video_frame.h>

#include "src/converters.h"
#include "src/converters/arguments.h"  // IWYU pragma: keep
#include "src/converters/interfaces.h"  // IWYU pragma: keep
#include "src/dictionaries/webrtc/video_frame.h"
#include "src/node/error.h"
#include "src/node/events.h"
#include "src/functional/validation.h"

// IWYU pragma: no_include <api/media_stream_interface.h>
// IWYU pragma: no_include <api/scoped_refptr.h>
// IWYU pragma: no_include <nan_callbacks_12_inl.h>

Nan::Persistent<v8::FunctionTemplate>& node_webrtc::RTCVideoSink::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

node_webrtc::RTCVideoSink::RTCVideoSink(rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
  : node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCVideoSink>("RTCVideoSink", *this)
  , _track(std::move(track)) {
  rtc::VideoSinkWants wants;
  _track->AddOrUpdateSink(this, wants);
}

NAN_METHOD(node_webrtc::RTCVideoSink::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCVideoSink.");
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(track, rtc::scoped_refptr<webrtc::VideoTrackInterface>);
  auto sink = new RTCVideoSink(track);
  sink->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(node_webrtc::RTCVideoSink::GetStopped) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<node_webrtc::RTCVideoSink>::Unwrap(info.Holder());
  info.GetReturnValue().Set(self->_stopped);
}

void node_webrtc::RTCVideoSink::Stop() {
  if (_track) {
    _stopped = true;
    _track->RemoveSink(this);
    _track = nullptr;
  }
  node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCVideoSink>::Stop();
}

NAN_METHOD(node_webrtc::RTCVideoSink::JsStop) {
  auto self = AsyncObjectWrapWithLoop<node_webrtc::RTCVideoSink>::Unwrap(info.Holder());
  self->Stop();
}

void node_webrtc::RTCVideoSink::OnFrame(const webrtc::VideoFrame& frame) {
  Dispatch(node_webrtc::CreateCallback<node_webrtc::RTCVideoSink>([this, frame]() {
    Nan::HandleScope scope;
    auto maybeValue = node_webrtc::From<v8::Local<v8::Value>>(frame);
    if (maybeValue.IsInvalid()) {
      // TODO(mroberts): Should raise an error; although this really shouldn't happen.
      return;
    }
    auto value = maybeValue.UnsafeFromValid();
    v8::Local<v8::Value> argv[1];
    argv[0] = value;
    MakeCallback("onframe", 1, argv);
  }));
}

void node_webrtc::RTCVideoSink::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  node_webrtc::RTCVideoSink::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCVideoSink").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("stopped").ToLocalChecked(), GetStopped, nullptr);
  Nan::SetPrototypeMethod(tpl, "stop", JsStop);
  exports->Set(Nan::New("RTCVideoSink").ToLocalChecked(), tpl->GetFunction());
}
