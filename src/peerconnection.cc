/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/peerconnection.h"

#include "webrtc/api/mediaconstraintsinterface.h"
#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/base/refcount.h"
#include "webrtc/modules/audio_device/include/fake_audio_device.h"

#include "src/common.h"
#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/v8.h"
#include "src/converters/webrtc.h"
#include "src/create-answer-observer.h"
#include "src/create-offer-observer.h"
#include "src/datachannel.h"
#include "src/rtcstatsresponse.h"
#include "src/set-local-description-observer.h"
#include "src/set-remote-description-observer.h"
#include "src/stats-observer.h"

using node_webrtc::DataChannelEvent;
using node_webrtc::ErrorEvent;
using node_webrtc::Event;
using node_webrtc::From;
using node_webrtc::GetStatsEvent;
using node_webrtc::IceConnectionStateChangeEvent;
using node_webrtc::IceEvent;
using node_webrtc::IceGatheringStateChangeEvent;
using node_webrtc::SdpEvent;
using node_webrtc::SignalingStateChangeEvent;
using node_webrtc::PeerConnection;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Integer;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Value;
using v8::Array;
using webrtc::SessionDescriptionInterface;

using RTCConfiguration = webrtc::PeerConnectionInterface::RTCConfiguration;
using RTCOfferAnswerOptions = webrtc::PeerConnectionInterface::RTCOfferAnswerOptions;

Nan::Persistent<Function>* PeerConnection::constructor = nullptr;
rtc::Thread* PeerConnection::_signalingThread;
rtc::Thread* PeerConnection::_workerThread;

//
// PeerConnection
//

PeerConnection::PeerConnection(RTCConfiguration configuration): EventLoop<PeerConnection>(*this) {
  _createOfferObserver = new rtc::RefCountedObject<CreateOfferObserver>(this);
  _createAnswerObserver = new rtc::RefCountedObject<CreateAnswerObserver>(this);
  _setLocalDescriptionObserver = new rtc::RefCountedObject<SetLocalDescriptionObserver>(this);
  _setRemoteDescriptionObserver = new rtc::RefCountedObject<SetRemoteDescriptionObserver>(this);

  webrtc::FakeConstraints constraints;

  constraints.AddOptional(
      webrtc::MediaConstraintsInterface::kEnableDtlsSrtp,
      webrtc::MediaConstraintsInterface::kValueTrue);

  // FIXME: crashes without these constraints, why?
  constraints.AddMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveAudio,
      webrtc::MediaConstraintsInterface::kValueFalse);
  constraints.AddMandatory(
      webrtc::MediaConstraintsInterface::kOfferToReceiveVideo,
      webrtc::MediaConstraintsInterface::kValueFalse);

  _audioDeviceModule = new webrtc::FakeAudioDeviceModule();

  _jinglePeerConnectionFactory = webrtc::CreatePeerConnectionFactory(
      _workerThread,
      _signalingThread,
      _audioDeviceModule,
      nullptr,
      nullptr);

  _jinglePeerConnection = _jinglePeerConnectionFactory->CreatePeerConnection(
      configuration,
      &constraints,
      nullptr,
      nullptr,
      this);
}

PeerConnection::~PeerConnection() {
  TRACE_CALL;
  _jinglePeerConnection = nullptr;
  _jinglePeerConnectionFactory = nullptr;
  TRACE_END;
}

void PeerConnection::HandleErrorEvent(const ErrorEvent<PeerConnection>& event) const {
  Nan::HandleScope scope;

  auto pc = this->handle();
  auto callback = Local<Function>::Cast(pc->Get(Nan::New("onerror").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
      Nan::Error(event.msg.c_str())
  };
  Nan::MakeCallback(pc, callback, 1, argv);
}

void PeerConnection::HandleSdpEvent(const SdpEvent& event) const {
  Nan::HandleScope scope;

  auto self = this->handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onsuccess").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
    Nan::New(event.desc.c_str()).ToLocalChecked()
  };
  Nan::MakeCallback(self, callback, 1, argv);
}

void PeerConnection::HandleGetStatsEvent(const GetStatsEvent& event) const {
  Nan::HandleScope scope;

  if (event.callback->IsEmpty()) {
    return;
  }

  Local<Value> cargv[] = {
      Nan::New<External>(const_cast<void*>(reinterpret_cast<const void*>(&event.reports)))
  };

  Local<Value> argv[] = {
      Nan::NewInstance(Nan::New(*RTCStatsResponse::constructor), 1, cargv).ToLocalChecked()
  };
  event.callback->Call(1, argv);
}

void PeerConnection::HandleVoidEvent() const {
  Nan::HandleScope scope;

  auto self = this->handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onsuccess").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Nan::MakeCallback(self, callback, 0, nullptr);
}

void PeerConnection::HandleSignalingStateChangeEvent(const SignalingStateChangeEvent& event) {
  Nan::HandleScope scope;

  if (event.state == webrtc::PeerConnectionInterface::kClosed) {
    this->Stop();
  }

  auto self = this->handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onsignalingstatechange").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
      Nan::New(event.state)
  };
  Nan::MakeCallback(self, callback, 1, argv);
}

void PeerConnection::HandleIceConnectionStateChangeEvent(const IceConnectionStateChangeEvent& event) const {
  Nan::HandleScope scope;

  auto self = this->handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("oniceconnectionstatechange").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
      Nan::New(event.state)
  };
  Nan::MakeCallback(self, callback, 1, argv);
}

void PeerConnection::HandleIceGatheringStateChangeEvent(const IceGatheringStateChangeEvent& event) const {
  Nan::HandleScope scope;

  auto self = this->handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onicegatheringstatechange").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
      Nan::New(event.state)
  };
  Nan::MakeCallback(self, callback, 1, argv);
}

void PeerConnection::HandleIceCandidateEvent(const IceEvent& event) const {
  Nan::HandleScope scope;

  auto self = this->handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("onicecandidate").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> argv[] = {
      Nan::New(event.candidate.c_str()).ToLocalChecked(),
      Nan::New(event.sdpMid.c_str()).ToLocalChecked(),
      Nan::New(event.sdpMLineIndex)
  };
  Nan::MakeCallback(self, callback, 3, argv);
}

void PeerConnection::HandleDataChannelEvent(const DataChannelEvent& event) const {
  Nan::HandleScope scope;

  auto self = this->handle();
  auto callback = Local<Function>::Cast(self->Get(Nan::New("ondatachannel").ToLocalChecked()));
  if (callback.IsEmpty()) {
    return;
  }

  Local<Value> cargv[] = {
      Nan::New<External>(reinterpret_cast<void*>(event.observer))
  };

  Local<Value> argv[] = {
      Nan::NewInstance(Nan::New(*DataChannel::constructor), 1, cargv).ToLocalChecked()
  };
  Nan::MakeCallback(self, callback, 1, argv);
}

void PeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  TRACE_CALL;
  Dispatch(SignalingStateChangeEvent::Create(new_state));
  TRACE_END;
}

void PeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  TRACE_CALL;
  Dispatch(IceConnectionStateChangeEvent::Create(new_state));
  TRACE_END;
}

void PeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
  TRACE_CALL;
  Dispatch(IceGatheringStateChangeEvent::Create(new_state));
  TRACE_END;
}

void PeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  TRACE_CALL;
  Dispatch(IceEvent::Create(candidate));
  TRACE_END;
}

void PeerConnection::OnDataChannel(webrtc::DataChannelInterface* jingle_data_channel) {
  TRACE_CALL;
  Dispatch(DataChannelEvent::Create(new DataChannelObserver(jingle_data_channel)));
  TRACE_END;
}

void PeerConnection::OnAddStream(webrtc::MediaStreamInterface*) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRemoveStream(webrtc::MediaStreamInterface*) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRenegotiationNeeded() {
  TRACE_CALL;
  TRACE_END;
}

NAN_METHOD(PeerConnection::New) {
  TRACE_CALL;
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the PeerConnection.");
  }

  auto maybeConfiguration = From<Maybe<RTCConfiguration>, Nan::NAN_METHOD_ARGS_TYPE>(info);
  if (maybeConfiguration.IsInvalid()) {
    auto error = maybeConfiguration.ToErrors()[0];
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked());
  }

  // Tell em whats up
  auto obj = new PeerConnection(maybeConfiguration.UnsafeFromValid().FromMaybe(RTCConfiguration()));
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;
  auto validationOptions = From<Maybe<RTCOfferOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
      [](const Maybe<RTCOfferOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCOfferOptions()); });
  if (validationOptions.IsInvalid()) {
    auto error = validationOptions.ToErrors()[0];
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked());
  }

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  if (self->_jinglePeerConnection) {
    self->_jinglePeerConnection->CreateOffer(self->_createOfferObserver, nullptr); // options.options);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;
  // NOTE(mroberts): We do the validation per the Web IDL here, but we don't
  // actually use the RTCAnswerOptions.
  auto validationOptions = From<Maybe<RTCAnswerOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
      [](const Maybe<RTCAnswerOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCAnswerOptions()); });
  if (validationOptions.IsInvalid()) {
    auto error = validationOptions.ToErrors()[0];
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked());
  }

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  if (self->_jinglePeerConnection) {
    self->_jinglePeerConnection->CreateAnswer(self->_createAnswerObserver, nullptr);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;
  auto maybeDescription = From<SessionDescriptionInterface*, Nan::NAN_METHOD_ARGS_TYPE>(info);
  if (maybeDescription.IsInvalid()) {
    auto error = maybeDescription.ToErrors()[0];
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked());
  }
  auto description = maybeDescription.UnsafeFromValid();

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  if (self->_jinglePeerConnection) {
    self->_jinglePeerConnection->SetLocalDescription(self->_setLocalDescriptionObserver, description);
  } else {
    delete description;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetRemoteDescription) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  if (self->_jinglePeerConnection != nullptr) {
    auto desc = Local<Object>::Cast(info[0]);
    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(
        *String::Utf8Value(desc->Get(Nan::New("type").ToLocalChecked())->ToString()),
        *String::Utf8Value(desc->Get(Nan::New("sdp").ToLocalChecked())->ToString()),
        &error);
    self->_jinglePeerConnection->SetRemoteDescription(self->_setRemoteDescriptionObserver, sdi);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::AddIceCandidate) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  auto sdp = Handle<Object>::Cast(info[0]);
  webrtc::SdpParseError sdpParseError;
  webrtc::IceCandidateInterface* ci = webrtc::CreateIceCandidate(
      *String::Utf8Value(sdp->Get(Nan::New("sdpMid").ToLocalChecked())->ToString()),
      sdp->Get(Nan::New("sdpMLineIndex").ToLocalChecked())->Uint32Value(),
      *String::Utf8Value(sdp->Get(Nan::New("candidate").ToLocalChecked())->ToString()),
      &sdpParseError);

  if (self->_jinglePeerConnection && self->_jinglePeerConnection->AddIceCandidate(ci)) {
    self->Dispatch(AddIceCandidateSuccessEvent::Create());
  } else {
    std::string error = std::string("Failed to set ICE candidate");
    if (!self->_jinglePeerConnection) {
      error += ", no jingle peer connection.";
    } else if (sdpParseError.description.length()) {
      error += std::string(", parse error: ") + sdpParseError.description + ".";
    }
    self->Dispatch(AddIceCandidateErrorEvent::Create(error));
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateDataChannel) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  if (!self->_jinglePeerConnection) {
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }

  String::Utf8Value label(info[0]->ToString());
  auto dataChannelDict = Handle<Object>::Cast(info[1]);

  webrtc::DataChannelInit dataChannelInit;
  if (dataChannelDict->Has(Nan::New("id").ToLocalChecked())) {
    auto value = dataChannelDict->Get(Nan::New("id").ToLocalChecked());
    if (value->IsInt32()) {
      dataChannelInit.id = value->Int32Value();
    }
  }
  if (dataChannelDict->Has(Nan::New("maxRetransmitTime").ToLocalChecked())) {
    auto value = dataChannelDict->Get(Nan::New("maxRetransmitTime").ToLocalChecked());
    if (value->IsInt32()) {
      dataChannelInit.maxRetransmitTime = value->Int32Value();
    }
  }
  if (dataChannelDict->Has(Nan::New("maxRetransmits").ToLocalChecked())) {
    auto value = dataChannelDict->Get(Nan::New("maxRetransmits").ToLocalChecked());
    if (value->IsInt32()) {
      dataChannelInit.maxRetransmits = value->Int32Value();
    }
  }
  if (dataChannelDict->Has(Nan::New("negotiated").ToLocalChecked())) {
    auto value = dataChannelDict->Get(Nan::New("negotiated").ToLocalChecked());
    if (value->IsBoolean()) {
      dataChannelInit.negotiated = value->BooleanValue();
    }
  }
  if (dataChannelDict->Has(Nan::New("ordered").ToLocalChecked())) {
    auto value = dataChannelDict->Get(Nan::New("ordered").ToLocalChecked());
    if (value->IsBoolean()) {
      dataChannelInit.ordered = value->BooleanValue();
    }
  }
  if (dataChannelDict->Has(Nan::New("protocol").ToLocalChecked())) {
    auto value = dataChannelDict->Get(Nan::New("protocol").ToLocalChecked());
    if (value->IsString()) {
      dataChannelInit.protocol = *String::Utf8Value(value->ToString());
    }
  }

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface =
      self->_jinglePeerConnection->CreateDataChannel(*label, &dataChannelInit);

  Local<Value> cargv[] = {
      Nan::New<External>(reinterpret_cast<void*>(new DataChannelObserver(data_channel_interface)))
  };
  auto dc = Nan::NewInstance(Nan::New(*DataChannel::constructor), 1, cargv).ToLocalChecked();

  TRACE_END;
  info.GetReturnValue().Set(dc);
}

NAN_METHOD(PeerConnection::GetStats) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  auto *onSuccess = new Nan::Callback(info[0].As<Function>());
  auto *onFailure = new Nan::Callback(info[1].As<Function>());
  rtc::scoped_refptr<StatsObserver> statsObserver =
     new rtc::RefCountedObject<StatsObserver>(self, onSuccess);

  if (!self->_jinglePeerConnection) {
    Local<Value> argv[] = {
      Nan::New("data channel is closed").ToLocalChecked()
    };
    onFailure->Call(1, argv);
  } else if (!self->_jinglePeerConnection->GetStats(statsObserver, nullptr,
    webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
    // TODO: Include error?
    Local<Value> argv[] = {
      Nan::Null()
    };
    onFailure->Call(1, argv);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::UpdateIce) {
  TRACE_CALL;
  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::Close) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  if (self->_jinglePeerConnection) {
    self->_jinglePeerConnection->Close();
  }

  self->_jinglePeerConnection = nullptr;
  self->_jinglePeerConnectionFactory = nullptr;

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(PeerConnection::GetLocalDescription) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  const webrtc::SessionDescriptionInterface* sdi = nullptr;

  if (self->_jinglePeerConnection) {
    sdi = self->_jinglePeerConnection->local_description();
  }

  Handle<Value> value;
  if (!sdi) {
    value = Nan::Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = Nan::New(sdp.c_str()).ToLocalChecked();
  }

  TRACE_END;
#if NODE_MAJOR_VERSION == 0
  info.GetReturnValue().Set(Nan::New(value));
#else
  info.GetReturnValue().Set(value);
#endif
}

NAN_GETTER(PeerConnection::GetRemoteDescription) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  const webrtc::SessionDescriptionInterface* sdi = nullptr;

  if (self->_jinglePeerConnection) {
    sdi = self->_jinglePeerConnection->remote_description();
  }

  Handle<Value> value;
  if (!sdi) {
    value = Nan::Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = Nan::New(sdp.c_str()).ToLocalChecked();
  }

  TRACE_END;
#if NODE_MAJOR_VERSION == 0
  info.GetReturnValue().Set(Nan::New(value));
#else
  info.GetReturnValue().Set(value);
#endif
}

NAN_GETTER(PeerConnection::GetSignalingState) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  auto state = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->signaling_state()
      : webrtc::PeerConnectionInterface::kClosed;
  TRACE_END;
  info.GetReturnValue().Set(Nan::New(state));
}

NAN_GETTER(PeerConnection::GetIceConnectionState) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  auto state = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : webrtc::PeerConnectionInterface::kIceConnectionClosed;
  TRACE_END;
  info.GetReturnValue().Set(Nan::New(state));
}

NAN_GETTER(PeerConnection::GetIceGatheringState) {
  TRACE_CALL;
  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  auto state = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_gathering_state()
      : webrtc::PeerConnectionInterface::kIceGatheringComplete;
  TRACE_END;
  info.GetReturnValue().Set(Nan::New(state));
}

NAN_SETTER(PeerConnection::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void PeerConnection::Init(rtc::Thread* signalingThread, rtc::Thread* workerThread, Handle<Object> exports) {
  _signalingThread = signalingThread;
  _workerThread = workerThread;

  auto tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnection").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "createOffer", CreateOffer);
  Nan::SetPrototypeMethod(tpl, "createAnswer", CreateAnswer);
  Nan::SetPrototypeMethod(tpl, "setLocalDescription", SetLocalDescription);
  Nan::SetPrototypeMethod(tpl, "setRemoteDescription", SetRemoteDescription);
  Nan::SetPrototypeMethod(tpl, "getStats", GetStats);
  Nan::SetPrototypeMethod(tpl, "updateIce", UpdateIce);
  Nan::SetPrototypeMethod(tpl, "addIceCandidate", AddIceCandidate);
  Nan::SetPrototypeMethod(tpl, "createDataChannel", CreateDataChannel);
  Nan::SetPrototypeMethod(tpl, "close", Close);

  Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New("localDescription").ToLocalChecked(),
      GetLocalDescription,
      ReadOnly);

  Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New("remoteDescription").ToLocalChecked(),
      GetRemoteDescription,
      ReadOnly);

  Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New("signalingState").ToLocalChecked(),
      GetSignalingState,
      ReadOnly);

  Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New("iceConnectionState").ToLocalChecked(),
      GetIceConnectionState,
      ReadOnly);

  Nan::SetAccessor(
      tpl->InstanceTemplate(),
      Nan::New("iceGatheringState").ToLocalChecked(),
      GetIceGatheringState,
      ReadOnly);

  constructor = new Nan::Persistent<Function>(tpl->GetFunction());
  exports->Set(Nan::New("PeerConnection").ToLocalChecked(), tpl->GetFunction());
}

void PeerConnection::Dispose() {
  delete constructor;
}
