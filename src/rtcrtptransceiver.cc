/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcrtptransceiver.h"

#include <webrtc/api/rtptransceiverinterface.h>  // IWYU pragma: keep
#include <webrtc/rtc_base/scoped_ref_ptr.h>

#include "src/error.h"
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/converters/v8.h"  // IWYU pragma: keep
#include "src/rtcrtpreceiver.h"  // IWYU pragma: keep
#include "src/rtcrtpsender.h"  // IWYU pragma: keep

using node_webrtc::AsyncObjectWrap;
using node_webrtc::RTCRtpTransceiver;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function>& RTCRtpTransceiver::constructor() {
  static Nan::Persistent<Function> constructor;
  return constructor;
}

Nan::Persistent<FunctionTemplate>& RTCRtpTransceiver::tpl() {
  static Nan::Persistent<FunctionTemplate> tpl;
  return tpl;
}

RTCRtpTransceiver::RTCRtpTransceiver(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface>&& transceiver)
  : AsyncObjectWrap("RTCRtpTransceiver")
  , _factory(std::move(factory))
  , _transceiver(std::move(transceiver)) {
}

RTCRtpTransceiver::~RTCRtpTransceiver() {
  wrap()->Release(this);
}

NAN_METHOD(RTCRtpTransceiver::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a RTCRtpTransceiver");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(Local<External>::Cast(info[0])->Value());
  auto transceiver = *static_cast<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>*>(Local<External>::Cast(info[1])->Value());

  auto obj = new RTCRtpTransceiver(std::move(factory), std::move(transceiver));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCRtpTransceiver::GetMid) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->mid(), result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCRtpTransceiver::GetSender) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  auto sender = RTCRtpSender::wrap()->GetOrCreate(self->_factory, self->_transceiver->sender());
  info.GetReturnValue().Set(sender->ToObject());
}

NAN_GETTER(RTCRtpTransceiver::GetReceiver) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  auto receiver = RTCRtpReceiver::wrap()->GetOrCreate(self->_factory, self->_transceiver->receiver());
  info.GetReturnValue().Set(receiver->ToObject());
}

NAN_GETTER(RTCRtpTransceiver::GetStopped) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->stopped(), result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCRtpTransceiver::GetDirection) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->direction(), result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_SETTER(RTCRtpTransceiver::SetDirection) {
  (void) property;

  auto self = RTCRtpTransceiver::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, direction, webrtc::RtpTransceiverDirection);

  self->_transceiver->SetDirection(direction);
}

NAN_GETTER(RTCRtpTransceiver::GetCurrentDirection) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->current_direction(), result, Local<Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCRtpTransceiver::Stop) {
  (void) info;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  self->_transceiver->Stop();
}

NAN_METHOD(RTCRtpTransceiver::SetCodecPreferences) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

node_webrtc::Wrap <
RTCRtpTransceiver*,
rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
std::shared_ptr<node_webrtc::PeerConnectionFactory>
> * RTCRtpTransceiver::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpTransceiver*,
  rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
  std::shared_ptr<node_webrtc::PeerConnectionFactory>
  > (RTCRtpTransceiver::Create);
  return wrap;
}

RTCRtpTransceiver* RTCRtpTransceiver::Create(
    std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  Nan::HandleScope scope;
  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<External>(static_cast<void*>(&transceiver));
  auto value = Nan::NewInstance(Nan::New(RTCRtpTransceiver::constructor()), 2, cargv).ToLocalChecked();
  return RTCRtpTransceiver::Unwrap(value);
}

void RTCRtpTransceiver::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  RTCRtpTransceiver::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCRtpTransceiver").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("mid").ToLocalChecked(), GetMid, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("sender").ToLocalChecked(), GetSender, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("receiver").ToLocalChecked(), GetReceiver, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("stopped").ToLocalChecked(), GetStopped, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("direction").ToLocalChecked(), GetDirection, SetDirection);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentDirection").ToLocalChecked(), GetCurrentDirection, nullptr);
  Nan::SetPrototypeMethod(tpl, "stop", Stop);
  Nan::SetPrototypeMethod(tpl, "setCodecPreferences", SetCodecPreferences);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCRtpTransceiver").ToLocalChecked(), tpl->GetFunction());
}
