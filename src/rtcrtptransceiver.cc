/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcrtptransceiver.h"

#include <iostream>

#include <webrtc/api/rtptransceiverinterface.h>  // IWYU pragma: keep
#include <webrtc/rtc_base/scoped_ref_ptr.h>

#include "src/bidimap.h"
#include "src/error.h"
#include "src/converters/v8.h"
#include "src/converters/webrtc.h"  // IWYU pragma: keep
#include "src/rtcrtpreceiver.h"
#include "src/rtcrtpsender.h"
#include "src/mediastreamtrack.h"

using node_webrtc::AsyncObjectWrap;
using node_webrtc::BidiMap;
using node_webrtc::RTCRtpTransceiver;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> RTCRtpTransceiver::constructor;

BidiMap<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>, RTCRtpTransceiver*> RTCRtpTransceiver::_transceivers;

RTCRtpTransceiver::RTCRtpTransceiver(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface>&& transceiver)
  : AsyncObjectWrap("RTCRtpTransceiver")
  , _factory(std::move(factory))
  , _transceiver(std::move(transceiver)) {
}

RTCRtpTransceiver::~RTCRtpTransceiver() {
  Release(this);
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
  auto sender = RTCRtpSender::GetOrCreate(self->_factory, self->_transceiver->sender());
  info.GetReturnValue().Set(sender->ToObject());
}

NAN_GETTER(RTCRtpTransceiver::GetReceiver) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  auto receiver = RTCRtpReceiver::GetOrCreate(self->_factory, self->_transceiver->receiver());
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

  auto self = AsyncObjectWrapWithLoop<RTCRtpTransceiver>::Unwrap(info.Holder());

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

RTCRtpTransceiver* RTCRtpTransceiver::GetOrCreate(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  return _transceivers.computeIfAbsent(transceiver, [&factory, &transceiver]() {
    Nan::HandleScope scope;
    Local<Value> cargv[2];
    cargv[0] = Nan::New<External>(static_cast<void*>(&factory));
    cargv[1] = Nan::New<External>(static_cast<void*>(&transceiver));
    auto value = Nan::NewInstance(Nan::New(RTCRtpTransceiver::constructor), 2, cargv).ToLocalChecked();
    return AsyncObjectWrapWithLoop<RTCRtpTransceiver>::Unwrap(value);
  });
}

void RTCRtpTransceiver::Release(RTCRtpTransceiver* transceiver) {
  _transceivers.reverseRemove(transceiver);
}

void RTCRtpTransceiver::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
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
  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCRtpTransceiver").ToLocalChecked(), tpl->GetFunction());
}
