/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcvideosource.h"

#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/media/base/videocapturer.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/converters/dictionaries.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"
#include "src/peerconnectionfactory.h"
#include "src/webrtc/fakevideocapturer.h"

Nan::Persistent<v8::Function>& node_webrtc::RTCVideoSource::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

NAN_METHOD(node_webrtc::RTCVideoSource::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct an RTCVideoSource.");
  }

  // TODO(mroberts): Again, we have some implicit factory we are threading around. How to handle?
  auto factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();

  std::unique_ptr<cricket::VideoCapturer> capturer(new node_webrtc::FakeVideoCapturer());
  auto source = factory->factory()->CreateVideoSource(std::move(capturer));

  auto instance = new RTCVideoSource(source);
  instance->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(node_webrtc::RTCVideoSource::GetStats) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  webrtc::VideoTrackSourceInterface::Stats stats = {};
  auto maybeStats = self->_source->GetStats(&stats)
      ? node_webrtc::MakeJust(stats)
      : node_webrtc::MakeNothing<webrtc::VideoTrackSourceInterface::Stats>();
  auto result = node_webrtc::From<v8::Local<v8::Value>>(maybeStats);
  info.GetReturnValue().Set(result.UnsafeFromValid());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetNeedsDenoising) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  auto needsDenoising = node_webrtc::From<v8::Local<v8::Value>>(self->_source->needs_denoising());
  info.GetReturnValue().Set(needsDenoising.UnsafeFromValid());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetIsScreencast) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  info.GetReturnValue().Set(self->_source->is_screencast());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetRemote) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  info.GetReturnValue().Set(self->_source->remote());
}

NAN_GETTER(node_webrtc::RTCVideoSource::GetState) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCVideoSource>(info.Holder());
  auto state = node_webrtc::From<v8::Local<v8::Value>>(self->_source->state());
  info.GetReturnValue().Set(state.UnsafeFromValid());
}

void node_webrtc::RTCVideoSource::Init(v8::Handle<v8::Object> exports) {
  auto tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RTCVideoSource").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "getStats", GetStats);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("needsDenoising").ToLocalChecked(), GetNeedsDenoising, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("isScreencast").ToLocalChecked(), GetIsScreencast, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remote").ToLocalChecked(), GetRemote, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("state").ToLocalChecked(), GetState, nullptr);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCVideoSource").ToLocalChecked(), tpl->GetFunction());
}
