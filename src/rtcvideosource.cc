/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcvideosource.h"

#include <webrtc/api/video/i420_buffer.h>  // IWYU pragma: keep
#include <webrtc/api/video/video_frame.h>

#include "src/converters.h"  // IWYU pragma: keep
#include "src/converters/arguments.h"  // IWYU pragma: keep
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/error.h"
#include "src/functional/maybe.h"  // IWYU pragma: keep
#include "src/mediastreamtrack.h"  // IWYU pragma: keep

Nan::Persistent<v8::Function>& node_webrtc::RTCVideoSource::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

node_webrtc::RTCVideoSource::RTCVideoSource() {
  _source = new rtc::RefCountedObject<RTCVideoTrackSource>();
}

node_webrtc::RTCVideoSource::RTCVideoSource(const node_webrtc::RTCVideoSourceInit init) {
  auto needsDenoising = init.needsDenoising
  .Map([](const bool needsDenoising) { return absl::optional<bool>(needsDenoising); })
  .FromMaybe(absl::optional<bool>());
  _source = new rtc::RefCountedObject<RTCVideoTrackSource>(init.isScreencast, needsDenoising);
}

NAN_METHOD(node_webrtc::RTCVideoSource::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCVideoSource.");
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(maybeInit, node_webrtc::Maybe<node_webrtc::RTCVideoSourceInit>);
  auto init = maybeInit.FromMaybe(node_webrtc::RTCVideoSourceInit());

  auto instance = new RTCVideoSource(init);
  instance->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(node_webrtc::RTCVideoSource::CreateTrack) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());

  // TODO(mroberts): Again, we have some implicit factory we are threading around. How to handle?
  auto factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
  auto track = factory->factory()->CreateVideoTrack(rtc::CreateRandomUuid(), self->_source);
  auto result = node_webrtc::MediaStreamTrack::wrap()->GetOrCreate(factory, track);

  info.GetReturnValue().Set(result->ToObject());
}

NAN_METHOD(node_webrtc::RTCVideoSource::OnFrame) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  CONVERT_ARGS_OR_THROW_AND_RETURN(buffer, rtc::scoped_refptr<webrtc::I420Buffer>);
  webrtc::VideoFrame::Builder builder;
  auto frame = builder.set_video_frame_buffer(buffer).build();
  self->_source->PushFrame(frame);
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetNeedsDenoising) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  auto needsDenoising = node_webrtc::From<v8::Local<v8::Value>>(self->_source->needs_denoising());
  info.GetReturnValue().Set(needsDenoising.UnsafeFromValid());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetIsScreencast) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  info.GetReturnValue().Set(self->_source->is_screencast());
}

void node_webrtc::RTCVideoSource::Init(v8::Handle<v8::Object> exports) {
  auto tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RTCVideoSource").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "createTrack", CreateTrack);
  Nan::SetPrototypeMethod(tpl, "onFrame", OnFrame);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("needsDenoising").ToLocalChecked(), GetNeedsDenoising, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("isScreencast").ToLocalChecked(), GetIsScreencast, nullptr);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCVideoSource").ToLocalChecked(), tpl->GetFunction());
}
