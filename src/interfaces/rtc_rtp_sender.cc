/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_sender.h"

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/interfaces.h"
#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/rtp_parameters.h"
#include "src/node/error.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/node/utility.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCRtpSender::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& RTCRtpSender::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

RTCRtpSender::RTCRtpSender(
    std::shared_ptr<PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpSenderInterface>&& sender)
  : AsyncObjectWrap("RTCRtpSender")
  , _factory(std::move(factory))
  , _sender(std::move(sender)) {
}

RTCRtpSender::~RTCRtpSender() {
  wrap()->Release(this);
}

NAN_METHOD(RTCRtpSender::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a RTCRtpSender");
  }

  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto sender = *static_cast<rtc::scoped_refptr<webrtc::RtpSenderInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new RTCRtpSender(std::move(factory), std::move(sender));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCRtpSender::GetTrack) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpSender>(info.Holder());
  v8::Local<v8::Value> result = Nan::Null();
  auto track = self->_sender->track();
  if (track) {
    result = MediaStreamTrack::wrap()->GetOrCreate(self->_factory, track)->ToObject();
  }
  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCRtpSender::GetTransport) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpSender>(info.Holder());
  v8::Local<v8::Value> result = Nan::Null();
  auto transport = self->_sender->dtls_transport();
  if (transport) {
    result = RTCDtlsTransport::wrap()->GetOrCreate(self->_factory, transport)->ToObject();
  }
  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCRtpSender::GetRtcpTransport) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(RTCRtpSender::GetCapabilities) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

NAN_METHOD(RTCRtpSender::GetParameters) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpSender>(info.Holder());
  auto parameters = self->_sender->GetParameters();
  CONVERT_OR_THROW_AND_RETURN(parameters, result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpSender::SetParameters) {
  RETURNS_PROMISE(resolver);
  Reject(resolver, Nan::Error("Not yet implemented; file a feature request against node-webrtc"));
}

NAN_METHOD(RTCRtpSender::GetStats) {
  RETURNS_PROMISE(resolver);
  Reject(resolver, Nan::Error("Not yet implemented; file a feature request against node-webrtc"));
}

NAN_METHOD(RTCRtpSender::ReplaceTrack) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpSender>(info.Holder());
  RETURNS_PROMISE(resolver);
  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, maybeTrack, Either<Null COMMA MediaStreamTrack*>);
  auto mediaStreamTrack = maybeTrack.FromEither<MediaStreamTrack*>([](auto) {
    return nullptr;
  }, [](auto track) {
    return track;
  });
  auto track = mediaStreamTrack ? mediaStreamTrack->track().get() : nullptr;
  self->_sender->SetTrack(track)
  ? Resolve(resolver, Nan::Undefined())
  : Reject(resolver, Nan::Error("Failed to replaceTrack"));
}

Wrap <
RTCRtpSender*,
rtc::scoped_refptr<webrtc::RtpSenderInterface>,
std::shared_ptr<PeerConnectionFactory>
> * RTCRtpSender::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpSender*,
  rtc::scoped_refptr<webrtc::RtpSenderInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > (RTCRtpSender::Create);
  return wrap;
}

RTCRtpSender* RTCRtpSender::Create(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpSenderInterface> sender) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&sender));
  auto value = Nan::NewInstance(Nan::New(RTCRtpSender::constructor()), 2, cargv).ToLocalChecked();
  return AsyncObjectWrapWithLoop<RTCRtpSender>::Unwrap(value);
}

void RTCRtpSender::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  RTCRtpSender::tpl().Reset(tpl);
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

CONVERT_INTERFACE_TO_AND_FROM_JS(RTCRtpSender, "RTCRtpSender", ToObject, AsyncObjectWrapWithLoop<RTCRtpSender>::Unwrap)

}  // namespace node_webrtc
