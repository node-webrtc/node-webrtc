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
#include <v8.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/dictionaries/webrtc/video_frame.h"  // IWYU pragma: keep
#include "src/interfaces/media_stream_track.h"  // IWYU pragma: keep
#include "src/node/events.h"
#include "src/functional/validation.h"

namespace node_webrtc {

Nan::Persistent<v8::FunctionTemplate>& RTCVideoSink::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

RTCVideoSink::RTCVideoSink(rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
  : AsyncObjectWrapWithLoop<RTCVideoSink>("RTCVideoSink", *this)
  , _track(std::move(track)) {
  rtc::VideoSinkWants wants;
  _track->AddOrUpdateSink(this, wants);
}

NAN_METHOD(RTCVideoSink::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCVideoSink.");
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(track, rtc::scoped_refptr<webrtc::VideoTrackInterface>);
  auto sink = new RTCVideoSink(track);
  sink->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCVideoSink::GetStopped) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<RTCVideoSink>::Unwrap(info.Holder());
  info.GetReturnValue().Set(self->_stopped);
}

void RTCVideoSink::Stop() {
  if (_track) {
    _stopped = true;
    _track->RemoveSink(this);
    _track = nullptr;
  }
  AsyncObjectWrapWithLoop<RTCVideoSink>::Stop();
}

NAN_METHOD(RTCVideoSink::JsStop) {
  auto self = AsyncObjectWrapWithLoop<RTCVideoSink>::Unwrap(info.Holder());
  self->Stop();
}

void RTCVideoSink::OnFrame(const webrtc::VideoFrame& frame) {
  Dispatch(CreateCallback<RTCVideoSink>([this, frame]() {
    Nan::HandleScope scope;
    auto maybeValue = From<v8::Local<v8::Value>>(frame);
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

void RTCVideoSink::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  RTCVideoSink::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCVideoSink").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("stopped").ToLocalChecked(), GetStopped, nullptr);
  Nan::SetPrototypeMethod(tpl, "stop", JsStop);
  exports->Set(Nan::New("RTCVideoSink").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace node_webrtc
