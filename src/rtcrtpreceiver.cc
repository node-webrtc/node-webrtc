/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcrtpreceiver.h"

#include <webrtc/api/rtpreceiverinterface.h>  // IWYU pragma: keep
#include <webrtc/rtc_base/scoped_ref_ptr.h>

#include "src/error.h"
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/mediastreamtrack.h"  // IWYU pragma: keep

using node_webrtc::AsyncObjectWrap;
using node_webrtc::RTCRtpReceiver;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function>& RTCRtpReceiver::constructor() {
  static Nan::Persistent<Function> constructor;
  return constructor;
}

Nan::Persistent<FunctionTemplate>& RTCRtpReceiver::tpl() {
  static Nan::Persistent<FunctionTemplate> tpl;
  return tpl;
}

RTCRtpReceiver::RTCRtpReceiver(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpReceiverInterface>&& receiver)
  : AsyncObjectWrap("RTCRtpReceiver")
  , _factory(std::move(factory))
  , _receiver(std::move(receiver)) {
}

RTCRtpReceiver::~RTCRtpReceiver() {
  wrap()->Release(this);
}

NAN_METHOD(RTCRtpReceiver::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a RTCRtpReceiver");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(Local<External>::Cast(info[0])->Value());
  auto receiver = *static_cast<rtc::scoped_refptr<webrtc::RtpReceiverInterface>*>(Local<External>::Cast(info[1])->Value());

  auto obj = new RTCRtpReceiver(std::move(factory), std::move(receiver));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCRtpReceiver::GetTrack) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpReceiver>(info.Holder());
  auto track = MediaStreamTrack::wrap()->GetOrCreate(self->_factory, self->_receiver->track());
  info.GetReturnValue().Set(track->ToObject());
}

NAN_GETTER(RTCRtpReceiver::GetTransport) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCRtpReceiver::GetRtcpTransport) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(RTCRtpReceiver::GetCapabilities) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

NAN_METHOD(RTCRtpReceiver::GetParameters) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpReceiver>(info.Holder());
  auto parameters = self->_receiver->GetParameters();
  CONVERT_OR_THROW_AND_RETURN(parameters, result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpReceiver::GetContributingSources) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpReceiver>(info.Holder());
  auto contributingSources = std::vector<webrtc::RtpSource>();
  auto sources = self->_receiver->GetSources();
  for (auto source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::CSRC) {
      contributingSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN(contributingSources, result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpReceiver::GetSynchronizationSources) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpReceiver>(info.Holder());
  auto synchronizationSources = std::vector<webrtc::RtpSource>();
  auto sources = self->_receiver->GetSources();
  for (auto source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::SSRC) {
      synchronizationSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN(synchronizationSources, result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpReceiver::GetStats) {
  auto resolver = v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked();
  resolver->Reject(Nan::GetCurrentContext(), Nan::Error("Not yet implemented; file a feature request against node-webrtc")).IsNothing();
  info.GetReturnValue().Set(resolver->GetPromise());
}

node_webrtc::Wrap <
RTCRtpReceiver*,
rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
std::shared_ptr<node_webrtc::PeerConnectionFactory>
> * RTCRtpReceiver::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpReceiver*,
  rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
  std::shared_ptr<node_webrtc::PeerConnectionFactory>
  > (RTCRtpReceiver::Create);
  return wrap;
}

RTCRtpReceiver* RTCRtpReceiver::Create(
    std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  Nan::HandleScope scope;
  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<External>(static_cast<void*>(&receiver));
  auto value = Nan::NewInstance(Nan::New(RTCRtpReceiver::constructor()), 2, cargv).ToLocalChecked();
  return RTCRtpReceiver::Unwrap(value);
}

void RTCRtpReceiver::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  RTCRtpReceiver::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCRtpReceiver").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("track").ToLocalChecked(), GetTrack, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("transport").ToLocalChecked(), GetTransport, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("rtcpTransport").ToLocalChecked(), GetRtcpTransport, nullptr);
  Nan::SetMethod(tpl, "getCapabilities", GetCapabilities);
  Nan::SetPrototypeMethod(tpl, "getParameters", GetParameters);
  Nan::SetPrototypeMethod(tpl, "getContributingSources", GetContributingSources);
  Nan::SetPrototypeMethod(tpl, "getSynchronizationSources", GetSynchronizationSources);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCRtpReceiver").ToLocalChecked(), tpl->GetFunction());
}
