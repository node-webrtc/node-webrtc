/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
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
#include "src/converters/webrtc.h"  // IWYU pragma: keep
#include "src/mediastreamtrack.h"

using node_webrtc::AsyncObjectWrap;
using node_webrtc::RTCRtpTransceiver;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Local;
using v8::Object;
using v8::Value;

Nan::Persistent<Function> RTCRtpTransceiver::constructor;

RTCRtpTransceiver::RTCRtpTransceiver(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface>&& transceiver)
  : AsyncObjectWrap("RTCRtpTransceiver")
  , _factory(std::move(factory))
  , _transceiver(std::move(transceiver)) {
  // Do we need to do something here?
}

RTCRtpTransceiver::~RTCRtpTransceiver() {
  // Do we need to do something here?
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
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCRtpTransceiver::GetSender) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCRtpTransceiver::GetReceiver) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCRtpTransceiver::GetStopped) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCRtpTransceiver::GetDirection) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCRtpTransceiver::GetCurrentDirection) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_METHOD(RTCRtpTransceiver::Stop) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

NAN_METHOD(RTCRtpTransceiver::SetCodecPreferences) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

void RTCRtpTransceiver::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RTCRtpTransceiver").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("mid").ToLocalChecked(), GetMid, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("sender").ToLocalChecked(), GetSender, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("receiver").ToLocalChecked(), GetReceiver, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("stopped").ToLocalChecked(), GetStopped, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("direction").ToLocalChecked(), GetDirection, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentDirection").ToLocalChecked(), GetCurrentDirection, nullptr);
  Nan::SetPrototypeMethod(tpl, "stop", Stop);
  Nan::SetPrototypeMethod(tpl, "setCodecPreferences", SetCodecPreferences);
  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCRtpTransceiver").ToLocalChecked(), tpl->GetFunction());
}
