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
#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/video_frame_buffer.h"
#include "src/node/error.h"
#include "src/functional/maybe.h"
#include "src/interfaces/media_stream_track.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCVideoSource::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

RTCVideoSource::RTCVideoSource() {
  _source = new rtc::RefCountedObject<RTCVideoTrackSource>();
}

RTCVideoSource::RTCVideoSource(const RTCVideoSourceInit init) {
  auto needsDenoising = init.needsDenoising
  .Map([](auto needsDenoising) { return absl::optional<bool>(needsDenoising); })
  .FromMaybe(absl::optional<bool>());
  _source = new rtc::RefCountedObject<RTCVideoTrackSource>(init.isScreencast, needsDenoising);
}

NAN_METHOD(RTCVideoSource::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCVideoSource.");
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(maybeInit, Maybe<RTCVideoSourceInit>)
  auto init = maybeInit.FromMaybe(RTCVideoSourceInit());

  auto instance = new RTCVideoSource(init);
  instance->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RTCVideoSource::CreateTrack) {
  auto self = Nan::ObjectWrap::Unwrap<RTCVideoSource>(info.Holder());

  // TODO(mroberts): Again, we have some implicit factory we are threading around. How to handle?
  auto factory = PeerConnectionFactory::GetOrCreateDefault();
  auto track = factory->factory()->CreateVideoTrack(rtc::CreateRandomUuid(), self->_source);
  auto result = MediaStreamTrack::wrap()->GetOrCreate(factory, track);

  info.GetReturnValue().Set(result->ToObject());
}

NAN_METHOD(RTCVideoSource::OnFrame) {
  auto self = Nan::ObjectWrap::Unwrap<RTCVideoSource>(info.Holder());
  CONVERT_ARGS_OR_THROW_AND_RETURN(buffer, rtc::scoped_refptr<webrtc::I420Buffer>)
  webrtc::VideoFrame::Builder builder;
  auto frame = builder.set_video_frame_buffer(buffer).build();
  self->_source->PushFrame(frame);
}

NAN_GETTER(RTCVideoSource::GetNeedsDenoising) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<RTCVideoSource>(info.Holder());
  auto needsDenoising = From<v8::Local<v8::Value>>(self->_source->needs_denoising());
  info.GetReturnValue().Set(needsDenoising.UnsafeFromValid());
}

NAN_GETTER(RTCVideoSource::GetIsScreencast) {
  (void) property;
  auto self = Nan::ObjectWrap::Unwrap<RTCVideoSource>(info.Holder());
  info.GetReturnValue().Set(self->_source->is_screencast());
}

void RTCVideoSource::Init(v8::Handle<v8::Object> exports) {
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

}  // namespace node_webrtc
