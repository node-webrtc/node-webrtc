/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_video_sink.h"

#include <type_traits>
#include <utility>

#include <webrtc/api/video/video_source_interface.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/napi.h"
#include "src/dictionaries/webrtc/video_frame.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/interfaces/media_stream_track.h"  // IWYU pragma: keep
#include "src/node/events.h"

namespace node_webrtc {

Napi::FunctionReference& RTCVideoSink::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCVideoSink::RTCVideoSink(const Napi::CallbackInfo& info)
  : AsyncObjectWrapWithLoop<RTCVideoSink>("RTCVideoSink", *this, info) {
  if (!info.IsConstructCall()) {
    Napi::TypeError::New(info.Env(), "Use the new operator to construct an RTCVideoSink.").ThrowAsJavaScriptException();
    return;
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN_VOID_NAPI(info, track, rtc::scoped_refptr<webrtc::VideoTrackInterface>)

  _track = std::move(track);

  rtc::VideoSinkWants wants;
  _track->AddOrUpdateSink(this, wants);
}

Napi::Value RTCVideoSink::GetStopped(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _stopped, result, Napi::Value)
  return result;
}

void RTCVideoSink::Stop() {
  if (_track) {
    _stopped = true;
    _track->RemoveSink(this);
    _track = nullptr;
  }
  AsyncObjectWrapWithLoop<RTCVideoSink>::Stop();
}

Napi::Value RTCVideoSink::JsStop(const Napi::CallbackInfo& info) {
  Stop();
  return info.Env().Undefined();
}

void RTCVideoSink::OnFrame(const webrtc::VideoFrame& frame) {
  Dispatch(CreateCallback<RTCVideoSink>([this, frame]() {
    auto env = Env();
    Napi::HandleScope scope(env);
    auto maybeValue = From<Napi::Value>(std::make_pair(env, frame));
    if (maybeValue.IsInvalid()) {
      // TODO(mroberts): Should raise an error; although this really shouldn't happen.
      return;
    }
    auto object = Napi::Object::New(env);
    object.Set("type", Napi::String::New(env, "frame"));
    object.Set("frame", maybeValue.UnsafeFromValid());
    MakeCallback("dispatchEvent", { object });
  }));
}

void RTCVideoSink::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "RTCVideoSink", {
    InstanceAccessor("stopped", &RTCVideoSink::GetStopped, nullptr),
    InstanceMethod("stop", &RTCVideoSink::JsStop)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCVideoSink", func);
}

}  // namespace node_webrtc
