/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcrtptransceiver.h"

#include <webrtc/api/rtptransceiverinterface.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>

#include "src/error.h"
#include "src/converters/dictionaries.h"
#include "src/converters/v8.h"
#include "src/rtcrtpreceiver.h"
#include "src/rtcrtpsender.h"

Nan::Persistent<v8::Function>& node_webrtc::RTCRtpTransceiver::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& node_webrtc::RTCRtpTransceiver::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

node_webrtc::RTCRtpTransceiver::RTCRtpTransceiver(
    std::shared_ptr<node_webrtc::PeerConnectionFactory>&& factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface>&& transceiver)
  : node_webrtc::AsyncObjectWrap("RTCRtpTransceiver")
  , _factory(std::move(factory))
  , _transceiver(std::move(transceiver)) {
}

node_webrtc::RTCRtpTransceiver::~RTCRtpTransceiver() {
  wrap()->Release(this);
}

NAN_METHOD(node_webrtc::RTCRtpTransceiver::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct a RTCRtpTransceiver");
  }

  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto transceiver = *static_cast<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new node_webrtc::RTCRtpTransceiver(std::move(factory), std::move(transceiver));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(node_webrtc::RTCRtpTransceiver::GetMid) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->mid(), result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::RTCRtpTransceiver::GetSender) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpTransceiver>(info.Holder());
  auto sender = RTCRtpSender::wrap()->GetOrCreate(self->_factory, self->_transceiver->sender());
  info.GetReturnValue().Set(sender->ToObject());
}

NAN_GETTER(node_webrtc::RTCRtpTransceiver::GetReceiver) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpTransceiver>(info.Holder());
  auto receiver = RTCRtpReceiver::wrap()->GetOrCreate(self->_factory, self->_transceiver->receiver());
  info.GetReturnValue().Set(receiver->ToObject());
}

NAN_GETTER(node_webrtc::RTCRtpTransceiver::GetStopped) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->stopped(), result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::RTCRtpTransceiver::GetDirection) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->direction(), result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_SETTER(node_webrtc::RTCRtpTransceiver::SetDirection) {
  (void) property;

  auto self = node_webrtc::RTCRtpTransceiver::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, direction, webrtc::RtpTransceiverDirection);

  self->_transceiver->SetDirection(direction);
}

NAN_GETTER(node_webrtc::RTCRtpTransceiver::GetCurrentDirection) {
  (void) property;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->current_direction(), result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(node_webrtc::RTCRtpTransceiver::Stop) {
  (void) info;
  auto self = node_webrtc::AsyncObjectWrap::Unwrap<node_webrtc::RTCRtpTransceiver>(info.Holder());
  self->_transceiver->Stop();
}

NAN_METHOD(node_webrtc::RTCRtpTransceiver::SetCodecPreferences) {
  (void) info;
  Nan::ThrowError("Not yet implemented; file a feature request against node-webrtc");
}

node_webrtc::Wrap <
node_webrtc::RTCRtpTransceiver*,
rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
std::shared_ptr<node_webrtc::PeerConnectionFactory>
> * node_webrtc::RTCRtpTransceiver::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  node_webrtc::RTCRtpTransceiver*,
  rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
  std::shared_ptr<node_webrtc::PeerConnectionFactory>
  > (node_webrtc::RTCRtpTransceiver::Create);
  return wrap;
}

node_webrtc::RTCRtpTransceiver* node_webrtc::RTCRtpTransceiver::Create(
    std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&transceiver));
  auto value = Nan::NewInstance(Nan::New(node_webrtc::RTCRtpTransceiver::constructor()), 2, cargv).ToLocalChecked();
  return node_webrtc::RTCRtpTransceiver::Unwrap(value);
}

void node_webrtc::RTCRtpTransceiver::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  node_webrtc::RTCRtpTransceiver::tpl().Reset(tpl);
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
