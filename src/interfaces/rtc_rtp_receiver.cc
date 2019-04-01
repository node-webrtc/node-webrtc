/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_receiver.h"

#include <webrtc/api/rtp_receiver_interface.h>

#include "src/converters.h"
#include "src/converters/interfaces.h"
#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/rtp_parameters.h"
#include "src/dictionaries/webrtc/rtp_source.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/node/utility.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCRtpReceiver::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& RTCRtpReceiver::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

RTCRtpReceiver::RTCRtpReceiver(
    std::shared_ptr<PeerConnectionFactory>&& factory,
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

  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto receiver = *static_cast<rtc::scoped_refptr<webrtc::RtpReceiverInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());

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
  auto self = AsyncObjectWrap::Unwrap<RTCRtpReceiver>(info.Holder());
  v8::Local<v8::Value> result = Nan::Null();
  auto transport = self->_receiver->dtls_transport();
  if (transport) {
    result = RTCDtlsTransport::wrap()->GetOrCreate(self->_factory, transport)->ToObject();
  }
  info.GetReturnValue().Set(result);
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
  CONVERT_OR_THROW_AND_RETURN(parameters, result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpReceiver::GetContributingSources) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpReceiver>(info.Holder());
  auto contributingSources = std::vector<webrtc::RtpSource>();
  auto sources = self->_receiver->GetSources();
  for (const auto& source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::CSRC) {
      contributingSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN(contributingSources, result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpReceiver::GetSynchronizationSources) {
  auto self = AsyncObjectWrap::Unwrap<RTCRtpReceiver>(info.Holder());
  auto synchronizationSources = std::vector<webrtc::RtpSource>();
  auto sources = self->_receiver->GetSources();
  for (const auto& source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::SSRC) {
      synchronizationSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN(synchronizationSources, result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpReceiver::GetStats) {
  RETURNS_PROMISE(resolver)
  Reject(resolver, Nan::Error("Not yet implemented; file a feature request against node-webrtc"));
}

Wrap <
RTCRtpReceiver*,
rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
std::shared_ptr<PeerConnectionFactory>
> * RTCRtpReceiver::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpReceiver*,
  rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > (RTCRtpReceiver::Create);
  return wrap;
}

RTCRtpReceiver* RTCRtpReceiver::Create(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&receiver));
  auto value = Nan::NewInstance(Nan::New(RTCRtpReceiver::constructor()), 2, cargv).ToLocalChecked();
  return RTCRtpReceiver::Unwrap(value);
}

void RTCRtpReceiver::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
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

CONVERT_INTERFACE_TO_JS(RTCRtpReceiver, "RTCRtpReceiver", ToObject)

}  // namespace node_webrtc
