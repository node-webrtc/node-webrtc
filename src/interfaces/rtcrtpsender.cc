/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtcrtpsender.h"

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/dictionaries.h"
#include "src/converters/interfaces.h"
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/interfaces/mediastreamtrack.h"
#include "src/interfaces/rtcdtlstransport.h"
#include "src/utility.h"

Nan::Persistent<v8::Function>& node_webrtc::RTCRtpSender::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& node_webrtc::RTCRtpSender::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

node_webrtc::RTCRtpSender::RTCRtpSender(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpSenderInterface>&& sender)
  : AsyncObjectWrap("RTCRtpSender")
  , _factory(std::move(factory))
  , _sender(std::move(sender)) {
}

node_webrtc::RTCRtpSender::~RTCRtpSender() {
  wrap()->Release(this);
}

NAN_METHOD(node_webrtc::RTCRtpSender::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a RTCRtpSender");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto sender = *static_cast<rtc::scoped_refptr<webrtc::RtpSenderInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new node_webrtc::RTCRtpSender(std::move(factory), std::move(sender));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(node_webrtc::RTCRtpSender::GetTrack) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpSender>(info.Holder());
  v8::Local<v8::Value> result = Nan::Null();
  auto track = self->_sender->track();
  if (track) {
    result = node_webrtc::MediaStreamTrack::wrap()->GetOrCreate(self->_factory, track)->ToObject();
  }
  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::RTCRtpSender::GetTransport) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpSender>(info.Holder());
  v8::Local<v8::Value> result = Nan::Null();
  auto transport = self->_sender->dtls_transport();
  if (transport) {
    result = node_webrtc::RTCDtlsTransport::wrap()->GetOrCreate(self->_factory, transport)->ToObject();
  }
  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::RTCRtpSender::GetRtcpTransport) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(node_webrtc::RTCRtpSender::GetCapabilities) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

NAN_METHOD(node_webrtc::RTCRtpSender::GetParameters) {
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpSender>(info.Holder());
  auto parameters = self->_sender->GetParameters();
  CONVERT_OR_THROW_AND_RETURN(parameters, result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(node_webrtc::RTCRtpSender::SetParameters) {
  RETURNS_PROMISE(resolver);
  node_webrtc::Reject(resolver, Nan::Error("Not yet implemented; file a feature request against node-webrtc"));
}

NAN_METHOD(node_webrtc::RTCRtpSender::GetStats) {
  RETURNS_PROMISE(resolver);
  node_webrtc::Reject(resolver, Nan::Error("Not yet implemented; file a feature request against node-webrtc"));
}

NAN_METHOD(node_webrtc::RTCRtpSender::ReplaceTrack) {
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpSender>(info.Holder());
  RETURNS_PROMISE(resolver);
  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, maybeTrack, node_webrtc::Either<node_webrtc::Null COMMA node_webrtc::MediaStreamTrack*>);
  auto mediaStreamTrack = maybeTrack.FromEither<node_webrtc::MediaStreamTrack*>([](Null) {
    return nullptr;
  }, [](node_webrtc::MediaStreamTrack * track) {
    return track;
  });
  auto track = mediaStreamTrack ? mediaStreamTrack->track().get() : nullptr;
  self->_sender->SetTrack(track)
  ? node_webrtc::Resolve(resolver, Nan::Undefined())
  : node_webrtc::Reject(resolver, Nan::Error("Failed to replaceTrack"));
}

node_webrtc::Wrap <
node_webrtc::RTCRtpSender*,
rtc::scoped_refptr<webrtc::RtpSenderInterface>,
std::shared_ptr<node_webrtc::PeerConnectionFactory>
> * node_webrtc::RTCRtpSender::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  node_webrtc::RTCRtpSender*,
  rtc::scoped_refptr<webrtc::RtpSenderInterface>,
  std::shared_ptr<node_webrtc::PeerConnectionFactory>
  > (node_webrtc::RTCRtpSender::Create);
  return wrap;
}

node_webrtc::RTCRtpSender* node_webrtc::RTCRtpSender::Create(
    std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpSenderInterface> sender) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&sender));
  auto value = Nan::NewInstance(Nan::New(node_webrtc::RTCRtpSender::constructor()), 2, cargv).ToLocalChecked();
  return node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCRtpSender>::Unwrap(value);
}

void node_webrtc::RTCRtpSender::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  node_webrtc::RTCRtpSender::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCRtpSender").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("track").ToLocalChecked(), GetTrack, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("transport").ToLocalChecked(), GetTransport, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("rtcpTransport").ToLocalChecked(), GetRtcpTransport, nullptr);
  Nan::SetMethod(tpl, "getCapabilities", GetCapabilities);
  Nan::SetPrototypeMethod(tpl, "getParameters", GetParameters);
  Nan::SetPrototypeMethod(tpl, "setParameters", SetParameters);
  Nan::SetPrototypeMethod(tpl, "getStats", GetStats);
  Nan::SetPrototypeMethod(tpl, "replaceTrack", ReplaceTrack);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCRtpSender").ToLocalChecked(), tpl->GetFunction());
}
