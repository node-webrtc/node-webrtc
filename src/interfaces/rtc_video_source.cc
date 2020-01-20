/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_video_source.h"

#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/video/i420_buffer.h>
#include <webrtc/api/video/video_frame.h>
#include <webrtc/rtc_base/ref_counted_object.h>

#include "src/converters.h"
#include "src/converters/absl.h"
#include "src/converters/arguments.h"
#include "src/converters/napi.h"
#include "src/dictionaries/webrtc/video_frame_buffer.h"
#include "src/functional/maybe.h"
#include "src/interfaces/media_stream_track.h"

#include <chrono>
#include <ctime>

namespace node_webrtc {

Napi::FunctionReference& RTCVideoSource::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCVideoSource::RTCVideoSource(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<RTCVideoSource>(info) {
  New(info);
}

Napi::Value RTCVideoSource::New(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  if (!info.IsConstructCall()) {
    Napi::TypeError::New(env, "Use the new operator to construct an RTCVideoSource.").ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, maybeInit, Maybe<RTCVideoSourceInit>)
  auto init = maybeInit.FromMaybe(RTCVideoSourceInit());

  auto needsDenoising = init.needsDenoising
  .Map([](auto needsDenoising) { return absl::optional<bool>(needsDenoising); })
  .FromMaybe(absl::optional<bool>());

  _source = new rtc::RefCountedObject<RTCVideoTrackSource>(init.isScreencast, needsDenoising);

  return info.Env().Undefined();
}

Napi::Value RTCVideoSource::CreateTrack(const Napi::CallbackInfo&) {
  // TODO(mroberts): Again, we have some implicit factory we are threading around. How to handle?
  auto factory = PeerConnectionFactory::GetOrCreateDefault();
  auto track = factory->factory()->CreateVideoTrack(rtc::CreateRandomUuid(), _source);
  return MediaStreamTrack::wrap()->GetOrCreate(factory, track)->Value();
}

Napi::Value RTCVideoSource::OnFrame(const Napi::CallbackInfo& info) {
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, buffer, rtc::scoped_refptr<webrtc::I420Buffer>)

  auto now = std::chrono::time_point_cast<std::chrono::microseconds>(std::chrono::system_clock::now());
  uint64_t nowInUs = now.time_since_epoch().count();

  webrtc::VideoFrame::Builder builder;
  auto frame = builder
      .set_timestamp_us(nowInUs)
      .set_video_frame_buffer(buffer)
      .build();
  _source->PushFrame(frame);
  return info.Env().Undefined();
}

Napi::Value RTCVideoSource::GetNeedsDenoising(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _source->needs_denoising(), result, Napi::Value)
  return result;
}

Napi::Value RTCVideoSource::GetIsScreencast(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _source->is_screencast(), result, Napi::Value)
  return result;
}

void RTCVideoSource::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "RTCVideoSource", {
    InstanceMethod("createTrack", &RTCVideoSource::CreateTrack),
    InstanceMethod("onFrame", &RTCVideoSource::OnFrame),
    InstanceAccessor("needsDenoising", &RTCVideoSource::GetNeedsDenoising, nullptr),
    InstanceAccessor("isScreencast", &RTCVideoSource::GetIsScreencast, nullptr)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCVideoSource", func);
}

}  // namespace node_webrtc
