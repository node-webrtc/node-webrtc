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
#include "createsessiondescriptionobserver.h"
#include "datachannel.h"
#include "error.h"
#include "functional/maybe.h"
#include "peerconnectionfactory.h"
#include "rtcrtpreceiver.h"
#include "rtcstatsresponse.h"
#include "setsessiondescriptionobserver.h"
#include "stats-observer.h"

using node_webrtc::Arguments;
using node_webrtc::AsyncObjectWrap;
using node_webrtc::AsyncObjectWrapWithLoop;
using node_webrtc::DataChannelEvent;
using node_webrtc::Event;
using node_webrtc::ExtendedRTCConfiguration;
using node_webrtc::From;
using node_webrtc::IceConnectionStateChangeEvent;
using node_webrtc::IceEvent;
using node_webrtc::IceGatheringStateChangeEvent;
using node_webrtc::Maybe;
using node_webrtc::MediaStreamTrack;
using node_webrtc::NegotiationNeededEvent;
using node_webrtc::OnAddTrackEvent;
using node_webrtc::PeerConnection;
using node_webrtc::PeerConnectionFactory;
using node_webrtc::PromiseEvent;
using node_webrtc::RTCRtpReceiver;
using node_webrtc::RTCRtpSender;
using node_webrtc::RTCSessionDescriptionInit;
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
  : AsyncObjectWrapWithLoop("RTCPeerConnection", *this) {

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
  for (auto receiver : _receivers) {
    receiver->RemoveRef();
  }
  _receivers.clear();
  for (auto sender : _senders) {
    sender->RemoveRef();
  }
  _senders.clear();
  if (_factory) {
    if (_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }
  TRACE_END;
}

void PeerConnection::HandleIceConnectionStateChangeEvent(const IceConnectionStateChangeEvent&) {
  TRACE_CALL;
  Nan::HandleScope scope;
  MakeCallback("oniceconnectionstatechange", 0, nullptr);
  MakeCallback("onconnectionstatechange", 0, nullptr);
  TRACE_END;
}

void PeerConnection::HandleIceGatheringStateChangeEvent(const IceGatheringStateChangeEvent&) {
  TRACE_CALL;
  Nan::HandleScope scope;
  MakeCallback("onicegatheringstatechange", 0, nullptr);
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
      MakeCallback("onicecandidate", 1, argv);
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
  Local<Value> dc = Nan::NewInstance(Nan::New(DataChannel::constructor), 1, cargv).ToLocalChecked();

  Local<Value> argv[1];
  argv[0] = dc;
  MakeCallback("ondatachannel", 1, argv);
  TRACE_END;
}

void PeerConnection::HandleNegotiationNeededEvent(const NegotiationNeededEvent&) {
  TRACE_CALL;
  Nan::HandleScope scope;
  MakeCallback("onnegotiationneeded", 0, nullptr);
  TRACE_END;
}

void PeerConnection::HandleOnAddTrackEvent(const OnAddTrackEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;

  auto rtpReceiver = event.receiver;

  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(static_cast<void*>(&_factory));
  cargv[1] = Nan::New<External>(static_cast<void*>(&rtpReceiver));
  auto receiver = RTCRtpReceiver::Unwrap(
          Nan::NewInstance(Nan::New(RTCRtpReceiver::constructor), 2, cargv).ToLocalChecked()
      );
  receiver->AddRef();
  _receivers.push_back(receiver);

  auto mediaStreams = std::vector<MediaStream*>();
  for (auto const& stream : event.streams) {
    auto mediaStream = MediaStream::GetOrCreate(_factory, stream);
    mediaStreams.push_back(mediaStream);
  }
  CONVERT_OR_THROW_AND_RETURN(mediaStreams, streams, Local<Value>);

  Local<Value> argv[2];
  argv[0] = receiver->ToObject();
  argv[1] = streams;
  MakeCallback("ontrack", 2, argv);

  TRACE_END;
}

void PeerConnection::HandleSignalingStateChangeEvent(const SignalingStateChangeEvent& event) {
  TRACE_CALL;
  Nan::HandleScope scope;
  MakeCallback("onsignalingstatechange", 0, nullptr);
  if (event.state == webrtc::PeerConnectionInterface::kClosed) {
    Stop();
  }
  TRACE_END;
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

void PeerConnection::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
  TRACE_CALL;
  Dispatch(OnAddTrackEvent::Create(receiver, streams));
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

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(PeerConnection::AddTrack) {
  TRACE_CALL;
  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError("Cannot addTrack; RTCPeerConnection is closed");
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<MediaStreamTrack* COMMA MediaStream*>);
  auto mediaStreamTrack = std::get<0>(pair);
  auto mediaStream = std::get<1>(pair);
  auto streams = std::vector<webrtc::MediaStreamInterface*>();
  streams.push_back(mediaStream->stream());
  auto rtpSender = self->_jinglePeerConnection->AddTrack(mediaStreamTrack->track(), streams);
  Local<Value> cargv[2];
  cargv[0] = Nan::New<External>(static_cast<void*>(&self->_factory));
  cargv[1] = Nan::New<External>(static_cast<void*>(&rtpSender));
  auto obj = Nan::NewInstance(Nan::New(RTCRtpSender::constructor), 2, cargv).ToLocalChecked();
  auto sender = AsyncObjectWrapWithLoop<RTCRtpSender>::Unwrap(obj);
  sender->AddRef();
  self->_senders.push_back(sender);
  TRACE_END;
  info.GetReturnValue().Set(sender->ToObject());
}

NAN_METHOD(PeerConnection::RemoveTrack) {
  TRACE_CALL;
  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError("Cannot removeTrack; RTCPeerConnection is closed");
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(sender, RTCRtpSender*);
  sender->RemoveRef();
  self->_senders.erase(std::find(self->_senders.begin(), self->_senders.end(), sender));
  self->_jinglePeerConnection->RemoveTrack(sender->sender());
  TRACE_END;
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

  SETUP_PROMISE(PeerConnection, RTCSessionDescriptionInit);

  auto validationOptions = From<Maybe<RTCOfferOptions>>(Arguments(info)).Map(
  [](const Maybe<RTCOfferOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCOfferOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    promise->Reject(SomeError(validationOptions.ToErrors()[0]));
    return;
  }

  auto options = validationOptions.UnsafeFromValid();

  if (self->_jinglePeerConnection != nullptr) {
    auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(self, std::move(promise));
    self->_jinglePeerConnection->CreateOffer(observer, options.options);
  }

  TRACE_END;
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

  SETUP_PROMISE(PeerConnection, RTCSessionDescriptionInit);

  auto validationOptions = From<Maybe<RTCAnswerOptions>>(Arguments(info)).Map(
  [](const Maybe<RTCAnswerOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCAnswerOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    promise->Reject(SomeError(validationOptions.ToErrors()[0]));
    return;
  }

  auto options = validationOptions.UnsafeFromValid();

  if (self->_jinglePeerConnection != nullptr) {
    auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(self, std::move(promise));
    self->_jinglePeerConnection->CreateAnswer(observer, options.options);
  }

  TRACE_END;
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

  SETUP_PROMISE(PeerConnection);

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, descriptionInit, RTCSessionDescriptionInit);

  if (descriptionInit.sdp.empty()) {
    descriptionInit.sdp = self->_lastSdp.sdp;
  }

  CONVERT_OR_REJECT_AND_RETURN(resolver, descriptionInit, description, SessionDescriptionInterface*);

  if (self->_jinglePeerConnection != nullptr) {
    auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(self, std::move(promise));
    self->_jinglePeerConnection->SetLocalDescription(observer, description);
  } else {
    delete description;
  }

  TRACE_END;
}

NAN_METHOD(PeerConnection::SetRemoteDescription) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

  SETUP_PROMISE(PeerConnection);

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, description, SessionDescriptionInterface*);

  if (self->_jinglePeerConnection != nullptr) {
    auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(self, std::move(promise));
    self->_jinglePeerConnection->SetRemoteDescription(observer, description);
  } else {
    delete description;
  }

  TRACE_END;
}

NAN_METHOD(PeerConnection::AddIceCandidate) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

  SETUP_PROMISE(PeerConnection);

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, candidate, IceCandidateInterface*);

  if (self->_jinglePeerConnection != nullptr && self->_jinglePeerConnection->AddIceCandidate(candidate)) {
    promise->Resolve(Undefined());
    self->Dispatch(std::move(promise));
  } else {
    std::string error = std::string("Failed to set ICE candidate");
    if (self->_jinglePeerConnection == nullptr) {
      error += ", no jingle peer connection";
    }
    error += ".";
    promise->Reject(SomeError(error));
    self->Dispatch(std::move(promise));
  }

  delete candidate;
  TRACE_END;
}

NAN_METHOD(PeerConnection::CreateDataChannel) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

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
  Local<Value> dc = Nan::NewInstance(Nan::New(DataChannel::constructor), 1, cargv).ToLocalChecked();

  TRACE_END;
  info.GetReturnValue().Set(dc);
}

NAN_METHOD(PeerConnection::GetConfiguration) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

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

NAN_METHOD(PeerConnection::GetReceivers) {
  TRACE_CALL;
  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());
  CONVERT_OR_THROW_AND_RETURN(self->_receivers, result, Local<Value>);
  info.GetReturnValue().Set(result);
  TRACE_END;
}

NAN_METHOD(PeerConnection::GetSenders) {
  TRACE_CALL;
  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());
  CONVERT_OR_THROW_AND_RETURN(self->_senders, result, Local<Value>);
  info.GetReturnValue().Set(result);
  TRACE_END;
}

NAN_METHOD(PeerConnection::GetStats) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

  SETUP_PROMISE(PeerConnection, RTCStatsResponseInit);

  if (self->_jinglePeerConnection != nullptr) {
    auto statsObserver = new rtc::RefCountedObject<StatsObserver>(self, std::move(promise));
    if (!self->_jinglePeerConnection->GetStats(statsObserver, nullptr,
            webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
      auto error = Nan::Error("Failed to execute getStats");
      resolver->Reject(error);
    }
  } else {
    auto error = Nan::Error("RTCPeerConnection is closed");
    resolver->Reject(error);
  }

  TRACE_END;
}

NAN_METHOD(PeerConnection::UpdateIce) {
  TRACE_CALL;
  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::Close) {
  TRACE_CALL;

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_cached_configuration = ExtendedRTCConfiguration(
            self->_jinglePeerConnection->GetConfiguration(),
            self->_port_range);
    self->_jinglePeerConnection->Close();
    for (auto receiver : self->_receivers) {
      receiver->OnPeerConnectionClosed();
    }
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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());

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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());
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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());
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

  auto self = AsyncObjectWrapWithLoop<PeerConnection>::Unwrap(info.Holder());
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

void PeerConnection::SaveLastSdp(const RTCSessionDescriptionInit& lastSdp) {
  this->_lastSdp = lastSdp;
}

void PeerConnection::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnection").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "addTrack", AddTrack);
  Nan::SetPrototypeMethod(tpl, "removeTrack", RemoveTrack);
  Nan::SetPrototypeMethod(tpl, "createOffer", CreateOffer);
  Nan::SetPrototypeMethod(tpl, "createAnswer", CreateAnswer);
  Nan::SetPrototypeMethod(tpl, "setLocalDescription", SetLocalDescription);
  Nan::SetPrototypeMethod(tpl, "setRemoteDescription", SetRemoteDescription);
  Nan::SetPrototypeMethod(tpl, "getConfiguration", GetConfiguration);
  Nan::SetPrototypeMethod(tpl, "setConfiguration", SetConfiguration);
  Nan::SetPrototypeMethod(tpl, "getReceivers", GetReceivers);
  Nan::SetPrototypeMethod(tpl, "getSenders", GetSenders);
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
