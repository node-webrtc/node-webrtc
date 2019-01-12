/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcvideosink.h"

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/dictionaries.h"
#include "src/converters/interfaces.h"
#include "peerconnectionfactory.h"

Nan::Persistent<v8::FunctionTemplate>& node_webrtc::RTCVideoSink::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

node_webrtc::RTCVideoSink::RTCVideoSink(rtc::scoped_refptr<webrtc::VideoTrackInterface> track)
  : node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCVideoSink>("RTCVideoSink", *this)
  , _track(std::move(track)) {
  rtc::VideoSinkWants wants;
  _track->AddOrUpdateSink(this, wants);
  // node_webrtc::PeerConnectionFactory::GetOrCreateDefault()->_signalingThread->Invoke<void>(RTC_FROM_HERE, [this]() {
  //   rtc::VideoSinkWants wants;
  //   this->_track->AddOrUpdateSink(this, wants);
  // });
}

NAN_METHOD(node_webrtc::RTCVideoSink::New) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(track, rtc::scoped_refptr<webrtc::VideoTrackInterface>);
  auto sink = new RTCVideoSink(track);
  sink->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(node_webrtc::RTCVideoSink::JsStop) {
  auto self = AsyncObjectWrapWithLoop<node_webrtc::RTCVideoSink>::Unwrap(info.Holder());
  self->Stop();
  if (self->_track) {
    self->_track->RemoveSink(self);
    self->_track = nullptr;
  }
}

void node_webrtc::RTCVideoSink::OnFrame(const webrtc::VideoFrame& frame) {
  Dispatch(node_webrtc::OnFrameEvent::Create(frame));
}

void node_webrtc::RTCVideoSink::HandleOnFrameEvent(const node_webrtc::OnFrameEvent& event) {
  Nan::HandleScope scope;
  auto maybeValue = node_webrtc::From<v8::Local<v8::Value>>(event.frame);
  if (maybeValue.IsInvalid()) {
    // TODO(mroberts): Should raise an error; although this really shouldn't happen.
    return;
  }
  auto value = maybeValue.UnsafeFromValid();
  v8::Local<v8::Value> argv[1];
  argv[0] = value;
  MakeCallback("onframe", 1, argv);
}

void node_webrtc::RTCVideoSink::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  node_webrtc::RTCVideoSink::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCVideoSink").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetPrototypeMethod(tpl, "stop", JsStop);
  exports->Set(Nan::New("RTCVideoSink").ToLocalChecked(), tpl->GetFunction());
}
