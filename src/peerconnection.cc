/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peerconnection.h"

#include "webrtc/base/refcountedobject.h"
#include "webrtc/p2p/client/basicportallocator.h"

#include "common.h"
#include "converters/arguments.h"
#include "converters/webrtc.h"
#include "create-answer-observer.h"
#include "create-offer-observer.h"
#include "datachannel.h"
#include "error.h"
#include "functional/maybe.h"
#include "peerconnectionfactory.h"
#include "rtcstatsresponse.h"
#include "set-local-description-observer.h"
#include "set-remote-description-observer.h"
#include "stats-observer.h"

using node_webrtc::DataChannelEvent;
using node_webrtc::ErrorEvent;
using node_webrtc::Event;
using node_webrtc::EventLoop;
using node_webrtc::ExtendedRTCConfiguration;
using node_webrtc::From;
using node_webrtc::GetStatsEvent;
using node_webrtc::IceConnectionStateChangeEvent;
using node_webrtc::IceEvent;
using node_webrtc::IceGatheringStateChangeEvent;
using node_webrtc::Maybe;
using node_webrtc::NegotiationNeededEvent;
using node_webrtc::PeerConnection;
using node_webrtc::PeerConnectionFactory;
using node_webrtc::RTCSessionDescriptionInit;
using node_webrtc::SdpEvent;
using node_webrtc::SignalingStateChangeEvent;
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
using webrtc::DataChannelInit;
using webrtc::IceCandidateInterface;
using webrtc::SessionDescriptionInterface;

using IceConnectionState = webrtc::PeerConnectionInterface::IceConnectionState;
using IceGatheringState = webrtc::PeerConnectionInterface::IceGatheringState;
using RTCConfiguration = webrtc::PeerConnectionInterface::RTCConfiguration;
using SignalingState = webrtc::PeerConnectionInterface::SignalingState;

Nan::Persistent<Function> PeerConnection::constructor;

//
// PeerConnection
//

PeerConnection::PeerConnection(ExtendedRTCConfiguration configuration)
  : Nan::AsyncResource("RTCPeerConnection")
  , EventLoop(*this) {
  _createOfferObserver = new rtc::RefCountedObject<CreateOfferObserver>(this);
  _createAnswerObserver = new rtc::RefCountedObject<CreateAnswerObserver>(this);
  _setLocalDescriptionObserver = new rtc::RefCountedObject<SetLocalDescriptionObserver>(this);
  _setRemoteDescriptionObserver = new rtc::RefCountedObject<SetRemoteDescriptionObserver>(this);

  // TODO(mroberts): Read `factory` (non-standard) from RTCConfiguration?
  _factory = PeerConnectionFactory::GetOrCreateDefault();
  _shouldReleaseFactory = true;

  auto portAllocator = std::unique_ptr<cricket::PortAllocator>(new cricket::BasicPortAllocator(
              _factory->getNetworkManager(),
              _factory->getSocketFactory()));
  _port_range = configuration.portRange;
  portAllocator->SetPortRange(
      _port_range.min.FromMaybe(0),
      _port_range.max.FromMaybe(65535));

  _jinglePeerConnection = _factory->factory()->CreatePeerConnection(
          configuration.configuration,
          std::move(portAllocator),
          nullptr,
          this);
}

PeerConnection::~PeerConnection() {
  TRACE_CALL;
  _jinglePeerConnection = nullptr;
  if (_factory) {
    if (_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }
  TRACE_END;
}

void PeerConnection::HandleErrorEvent(const ErrorEvent<PeerConnection>& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  Local<Value> argv[1];
  argv[0] = Nan::Error(event.msg.c_str());
  runInAsyncScope(handle(), "onerror", 1, argv);
  TRACE_END;
}

void PeerConnection::HandleGetStatsEvent(const GetStatsEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(const_cast<void*>(static_cast<const void*>(&event.timestamp)));
  cargv[1] = Nan::New<External>(const_cast<void*>(static_cast<const void*>(&event.reports)));
  Local<Value> argv[1];
  argv[0] = Nan::New(RTCStatsResponse::constructor)->NewInstance(2, cargv);
  runInAsyncScope(handle(), **event.callback, 1, argv);
  TRACE_END;
}

void PeerConnection::HandleIceConnectionStateChangeEvent(const IceConnectionStateChangeEvent&) {
  TRACE_CALL;
  Nan::HandleScope scope;
  runInAsyncScope(handle(), "oniceconnectionstatechange", 0, nullptr);
  runInAsyncScope(handle(), "onconnectionstatechange", 0, nullptr);
  TRACE_END;
}

void PeerConnection::HandleIceGatheringStateChangeEvent(const IceGatheringStateChangeEvent&) {
  TRACE_CALL;
  Nan::HandleScope scope;
  runInAsyncScope(handle(), "onicegatheringstatechange", 0, nullptr);
  TRACE_END;
}

void PeerConnection::HandleIceCandidateEvent(const IceEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  if (event.error.empty()) {
    auto maybeCandidate = From<Local<Value>>(event.candidate.get());
    if (maybeCandidate.IsValid()) {
      Local<Value> argv[1];
      argv[0] = maybeCandidate.UnsafeFromValid();
      runInAsyncScope(handle(), "onicecandidate", 1, argv);
    }
  }
  TRACE_END;
}

void PeerConnection::HandleDataChannelEvent(const DataChannelEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  DataChannelObserver* observer = event.observer;
  Local<Value> cargv[1];
  cargv[0] = Nan::New<External>(static_cast<void*>(observer));
  Local<Value> dc = Nan::New(DataChannel::constructor)->NewInstance(1, cargv);

  Local<Value> argv[1];
  argv[0] = dc;
  runInAsyncScope(handle(), "ondatachannel", 1, argv);
  TRACE_END;
}

void PeerConnection::HandleNegotiationNeededEvent(const NegotiationNeededEvent&) {
  TRACE_CALL;
  Nan::HandleScope scope;
  runInAsyncScope(handle(), "onnegotiationneeded", 0, nullptr);
  TRACE_END;
}

void PeerConnection::HandleSdpEvent(const SdpEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  _lastSdp = event.desc;
  Local<Value> argv[1];
  argv[0] = Nan::New(event.desc.c_str()).ToLocalChecked();
  runInAsyncScope(handle(), "onsuccess", 1, argv);
  TRACE_END;
}

void PeerConnection::HandleSignalingStateChangeEvent(const SignalingStateChangeEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  runInAsyncScope(handle(), "onsignalingstatechange", 0, nullptr);
  if (event.state == webrtc::PeerConnectionInterface::kClosed) {
    Stop();
  }
  TRACE_END;
}

void PeerConnection::HandleVoidEvent() {
  TRACE_CALL;
  Nan::HandleScope scope;
  runInAsyncScope(handle(), "onsuccess", 0, nullptr);
  TRACE_END;
}

void PeerConnection::DidStop() {
  Unref();
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

void PeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> jingle_data_channel) {
  TRACE_CALL;
  Dispatch(DataChannelEvent::Create(new DataChannelObserver(_factory, jingle_data_channel)));
  TRACE_END;
}

void PeerConnection::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRenegotiationNeeded() {
  TRACE_CALL;
  Dispatch(NegotiationNeededEvent::Create());
  TRACE_END;
}

NAN_METHOD(PeerConnection::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the PeerConnection.");
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, Maybe<ExtendedRTCConfiguration>);

  // Tell em whats up
  auto obj = new PeerConnection(configuration.FromMaybe(ExtendedRTCConfiguration()));
  obj->Wrap(info.This());
  obj->Ref();

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;

  auto validationOptions = From<Maybe<RTCOfferOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
  [](const Maybe<RTCOfferOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCOfferOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(validationOptions.ToErrors()[0]).ToLocalChecked());
    return;
  }

  auto options = validationOptions.UnsafeFromValid();

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateOffer(self->_createOfferObserver, options.options);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;

  auto validationOptions = From<Maybe<RTCAnswerOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
  [](const Maybe<RTCAnswerOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCAnswerOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(validationOptions.ToErrors()[0]).ToLocalChecked());
    return;
  }

  auto options = validationOptions.UnsafeFromValid();

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateAnswer(self->_createAnswerObserver, options.options);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;

  CONVERT_ARGS_OR_THROW_AND_RETURN(descriptionInit, RTCSessionDescriptionInit);

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (descriptionInit.sdp.empty()) {
    descriptionInit.sdp = self->_lastSdp;
  }

  CONVERT_OR_THROW_AND_RETURN(descriptionInit, description, SessionDescriptionInterface*);

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->SetLocalDescription(self->_setLocalDescriptionObserver, description);
  } else {
    delete description;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetRemoteDescription) {
  TRACE_CALL;

  CONVERT_ARGS_OR_THROW_AND_RETURN(description, SessionDescriptionInterface*);

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->SetRemoteDescription(self->_setRemoteDescriptionObserver, description);
  } else {
    delete description;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::AddIceCandidate) {
  TRACE_CALL;

  CONVERT_ARGS_OR_THROW_AND_RETURN(candidate, IceCandidateInterface*);

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr && self->_jinglePeerConnection->AddIceCandidate(candidate)) {
    self->Dispatch(AddIceCandidateSuccessEvent::Create());
  } else {
    delete candidate;
    std::string error = std::string("Failed to set ICE candidate");
    if (self->_jinglePeerConnection == nullptr) {
      error += ", no jingle peer connection";
    }
    error += ".";
    self->Dispatch(AddIceCandidateErrorEvent::Create(error));
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateDataChannel) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection == nullptr) {
    TRACE_END;
    Nan::ThrowError("Failed to execute 'createDataChannel' on 'RTCPeerConnection': The RTCPeerConnection's signalingState is 'closed'.");
    return;
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(args, std::tuple<std::string COMMA Maybe<DataChannelInit>>);

  auto label = std::get<0>(args);
  auto dataChannelInit = std::get<1>(args).FromMaybe(DataChannelInit());

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface =
      self->_jinglePeerConnection->CreateDataChannel(label, &dataChannelInit);

  auto observer = new DataChannelObserver(self->_factory, data_channel_interface);

  Local<Value> cargv[1];
  cargv[0] = Nan::New<External>(static_cast<void*>(observer));
  Local<Value> dc = Nan::New(DataChannel::constructor)->NewInstance(1, cargv);

  TRACE_END;
  info.GetReturnValue().Set(dc);
}

NAN_METHOD(PeerConnection::GetConfiguration) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? ExtendedRTCConfiguration(self->_jinglePeerConnection->GetConfiguration(), self->_port_range)
      : self->_cached_configuration,
      configuration,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(configuration);
}

NAN_METHOD(PeerConnection::SetConfiguration) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, RTCConfiguration);

  if (!self->_jinglePeerConnection) {
    TRACE_END;
    Nan::ThrowError("RTCPeerConnection is closed");
    return;
  }

  webrtc::RTCError rtcError;
  if (!self->_jinglePeerConnection->SetConfiguration(configuration, &rtcError)) {
    CONVERT_OR_THROW_AND_RETURN(&rtcError, error, Local<Value>);
    Nan::ThrowError(error);
    return;
  }

  TRACE_END;
}

NAN_METHOD(PeerConnection::GetStats) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  auto onSuccess = new Nan::Callback(info[0].As<Function>());
  auto onFailure = info[1].As<Function>();
  rtc::scoped_refptr<StatsObserver> statsObserver =
      new rtc::RefCountedObject<StatsObserver>(self, onSuccess);

  if (self->_jinglePeerConnection == nullptr) {
    Local<Value> argv[] = {
      Nan::New("data channel is closed").ToLocalChecked()
    };
    self->runInAsyncScope(self->handle(), onFailure, 1, argv);
  } else if (!self->_jinglePeerConnection->GetStats(statsObserver, nullptr,
          webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
    // TODO: Include error?
    Local<Value> argv[] = {
      Nan::Null()
    };
    self->runInAsyncScope(self->handle(), onFailure, 1, argv);
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

  if (self->_jinglePeerConnection != nullptr) {
    self->_cached_configuration = ExtendedRTCConfiguration(
            self->_jinglePeerConnection->GetConfiguration(),
            self->_port_range);
    self->_jinglePeerConnection->Close();
  }

  self->_jinglePeerConnection = nullptr;

  if (self->_factory) {
    if (self->_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    self->_factory = nullptr;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(PeerConnection::GetCanTrickleIceCandidates) {
  TRACE_CALL;
  (void) property;

  TRACE_END;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(PeerConnection::GetConnectionState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  auto iceConnectionState = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed;

  CONVERT_OR_THROW_AND_RETURN(iceConnectionState, connectionState, RTCPeerConnectionState);
  CONVERT_OR_THROW_AND_RETURN(connectionState, value, Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(value);
}

NAN_GETTER(PeerConnection::GetCurrentLocalDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_local_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetLocalDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->local_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetPendingLocalDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_local_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetCurrentRemoteDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_remote_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetRemoteDescription) {
  TRACE_CALL;

  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->remote_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetPendingRemoteDescription) {
  TRACE_CALL;

  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_remote_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetSignalingState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->signaling_state()
      : SignalingState::kClosed,
      state,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
}

NAN_GETTER(PeerConnection::GetIceConnectionState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : IceConnectionState::kIceConnectionClosed,
      state,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
}

NAN_GETTER(PeerConnection::GetIceGatheringState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_gathering_state()
      : IceGatheringState::kIceGatheringComplete,
      state,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
}

NAN_SETTER(PeerConnection::ReadOnly) {
  (void) info;
  (void) property;
  (void) value;
  INFO("PeerConnection::ReadOnly");
}

void PeerConnection::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnection").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "createOffer", CreateOffer);
  Nan::SetPrototypeMethod(tpl, "createAnswer", CreateAnswer);
  Nan::SetPrototypeMethod(tpl, "setLocalDescription", SetLocalDescription);
  Nan::SetPrototypeMethod(tpl, "setRemoteDescription", SetRemoteDescription);
  Nan::SetPrototypeMethod(tpl, "getConfiguration", GetConfiguration);
  Nan::SetPrototypeMethod(tpl, "setConfiguration", SetConfiguration);
  Nan::SetPrototypeMethod(tpl, "getStats", GetStats);
  Nan::SetPrototypeMethod(tpl, "updateIce", UpdateIce);
  Nan::SetPrototypeMethod(tpl, "addIceCandidate", AddIceCandidate);
  Nan::SetPrototypeMethod(tpl, "createDataChannel", CreateDataChannel);
  Nan::SetPrototypeMethod(tpl, "close", Close);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("canTrickleIceCandidates").ToLocalChecked(), GetCanTrickleIceCandidates, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("connectionState").ToLocalChecked(), GetConnectionState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentLocalDescription").ToLocalChecked(), GetCurrentLocalDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("localDescription").ToLocalChecked(), GetLocalDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingLocalDescription").ToLocalChecked(), GetPendingLocalDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentRemoteDescription").ToLocalChecked(), GetCurrentRemoteDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remoteDescription").ToLocalChecked(), GetRemoteDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingRemoteDescription").ToLocalChecked(), GetPendingRemoteDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("signalingState").ToLocalChecked(), GetSignalingState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceConnectionState").ToLocalChecked(), GetIceConnectionState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceGatheringState").ToLocalChecked(), GetIceGatheringState, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("PeerConnection").ToLocalChecked(), tpl->GetFunction());
}
