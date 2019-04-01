/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_transceiver.h"

#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/scoped_refptr.h>

#include "src/converters/absl.h"
#include "src/converters/interfaces.h"
#include "src/converters/v8.h"
#include "src/enums/webrtc/rtp_transceiver_direction.h"
#include "src/interfaces/rtc_rtp_receiver.h"
#include "src/interfaces/rtc_rtp_sender.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCRtpTransceiver::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& RTCRtpTransceiver::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

RTCRtpTransceiver::RTCRtpTransceiver(
    std::shared_ptr<PeerConnectionFactory>&& factory,
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

  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto transceiver = *static_cast<rtc::scoped_refptr<webrtc::RtpTransceiverInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new RTCRtpTransceiver(std::move(factory), std::move(transceiver));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCRtpTransceiver::GetMid) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->mid(), result, v8::Local<v8::Value>)
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
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->stopped(), result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCRtpTransceiver::GetDirection) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->direction(), result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_SETTER(RTCRtpTransceiver::SetDirection) {
  (void) property;

  auto self = RTCRtpTransceiver::Unwrap(info.Holder());

  CONVERT_OR_THROW_AND_RETURN(value, direction, webrtc::RtpTransceiverDirection)

  self->_transceiver->SetDirection(direction);
}

NAN_GETTER(RTCRtpTransceiver::GetCurrentDirection) {
  (void) property;
  auto self = AsyncObjectWrap::Unwrap<RTCRtpTransceiver>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_transceiver->current_direction(), result, v8::Local<v8::Value>)
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

Wrap <
RTCRtpTransceiver*,
rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
std::shared_ptr<PeerConnectionFactory>
> * RTCRtpTransceiver::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpTransceiver*,
  rtc::scoped_refptr<webrtc::RtpTransceiverInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > (RTCRtpTransceiver::Create);
  return wrap;
}

RTCRtpTransceiver* RTCRtpTransceiver::Create(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&transceiver));
  auto value = Nan::NewInstance(Nan::New(RTCRtpTransceiver::constructor()), 2, cargv).ToLocalChecked();
  return RTCRtpTransceiver::Unwrap(value);
}

void RTCRtpTransceiver::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
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

CONVERT_INTERFACE_TO_JS(RTCRtpTransceiver, "RTCRtpTransceiver", ToObject)

}  // namespace node_webrtc
