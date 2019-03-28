/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/peerconnection.h"

#include <iosfwd>

#include <webrtc/api/media_types.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/rtc_error.h>
#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/p2p/client/basic_port_allocator.h>

#include "src/asyncobjectwrapwithloop.h"
#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/dictionaries.h"
#include "src/converters/enums.h"
#include "src/converters/interfaces.h"
#include "src/converters/v8.h"
#include "src/createsessiondescriptionobserver.h"
#include "src/datachannel.h"
#include "src/error.h"
#include "src/errorfactory.h"
#include "src/events.h"
#include "src/functional/either.h"
#include "src/functional/maybe.h"
#include "src/mediastream.h"
#include "src/mediastreamtrack.h"
#include "src/peerconnectionfactory.h"
#include "src/promise.h"
#include "src/rtcrtpreceiver.h"
#include "src/rtcrtpsender.h"
#include "src/rtcrtptransceiver.h"
#include "src/rtcstatscollector.h"
#include "src/setsessiondescriptionobserver.h"
#include "src/stats-observer.h"
#include "src/utility.h"

namespace webrtc {

class SessionDescriptionInterface;
struct DataChannelInit;

}  // namespace webrtc

Nan::Persistent<v8::Function>& node_webrtc::PeerConnection::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

//
// PeerConnection
//

node_webrtc::PeerConnection::PeerConnection(node_webrtc::ExtendedRTCConfiguration configuration)
  : node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>("RTCPeerConnection", *this) {

  // TODO(mroberts): Read `factory` (non-standard) from RTCConfiguration?
  _factory = node_webrtc::PeerConnectionFactory::GetOrCreateDefault();
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

node_webrtc::PeerConnection::~PeerConnection() {
  _jinglePeerConnection = nullptr;
  _channels.clear();
  if (_factory) {
    if (_shouldReleaseFactory) {
      node_webrtc::PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }
}

void node_webrtc::PeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state) {
  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this, state]() {
    Nan::HandleScope scope;
    MakeCallback("onsignalingstatechange", 0, nullptr);
    if (state == webrtc::PeerConnectionInterface::kClosed) {
      Stop();
    }
  }));
}

void node_webrtc::PeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState) {
  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this]() {
    Nan::HandleScope scope;
    MakeCallback("oniceconnectionstatechange", 0, nullptr);
    MakeCallback("onconnectionstatechange", 0, nullptr);
  }));
}

void node_webrtc::PeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState) {
  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this]() {
    Nan::HandleScope scope;
    MakeCallback("onicegatheringstatechange", 0, nullptr);
  }));
}

void node_webrtc::PeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* ice_candidate) {
  std::string error;

  std::string sdp;
  if (!ice_candidate->ToString(&sdp)) {
    error = "Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/js-platform/node-webrtc";
    return;
  }

  webrtc::SdpParseError parseError;
  auto candidate = std::shared_ptr<webrtc::IceCandidateInterface>(webrtc::CreateIceCandidate(
              ice_candidate->sdp_mid(),
              ice_candidate->sdp_mline_index(),
              sdp,
              &parseError));
  if (!parseError.description.empty()) {
    error = parseError.description;
  } else if (!candidate) {
    error = "Failed to copy RTCIceCandidate";
  }

  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this, candidate, error]() {
    Nan::HandleScope scope;
    if (error.empty()) {
      auto maybeCandidate = node_webrtc::From<v8::Local<v8::Value>>(candidate.get());
      if (maybeCandidate.IsValid()) {
        v8::Local<v8::Value> argv[1];
        argv[0] = maybeCandidate.UnsafeFromValid();
        MakeCallback("onicecandidate", 1, argv);
      }
    }
  }));
}

void node_webrtc::PeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
  auto observer = new DataChannelObserver(_factory, channel);
  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this, observer]() {
    Nan::HandleScope scope;
    auto channel = DataChannel::wrap()->GetOrCreate(observer, observer->channel());
    v8::Local<v8::Value> argv = channel->ToObject();
    MakeCallback("ondatachannel", 1, &argv);
  }));
}

void node_webrtc::PeerConnection::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) {
}

void node_webrtc::PeerConnection::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
  if (_jinglePeerConnection->GetConfiguration().sdp_semantics != webrtc::SdpSemantics::kPlanB) {
    return;
  }
  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this, receiver, streams]() {
    Nan::HandleScope scope;

    auto mediaStreams = std::vector<MediaStream*>();
    for (auto const& stream : streams) {
      auto mediaStream = MediaStream::wrap()->GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN(mediaStreams, streamArray, v8::Local<v8::Value>);

    v8::Local<v8::Value> argv[3];
    argv[0] = node_webrtc::RTCRtpReceiver::wrap()->GetOrCreate(_factory, receiver)->ToObject();
    argv[1] = streamArray;
    argv[2] = Nan::Null();
    MakeCallback("ontrack", 3, argv);
  }));
}

void node_webrtc::PeerConnection::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  auto receiver = transceiver->receiver();
  auto streams = receiver->streams();
  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this, transceiver, receiver, streams]() {
    Nan::HandleScope scope;

    auto mediaStreams = std::vector<MediaStream*>();
    for (auto const& stream : streams) {
      auto mediaStream = MediaStream::wrap()->GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN(mediaStreams, streamArray, v8::Local<v8::Value>);

    v8::Local<v8::Value> argv[3];
    argv[0] = node_webrtc::RTCRtpReceiver::wrap()->GetOrCreate(_factory, receiver)->ToObject();
    argv[1] = streamArray;
    argv[2] = node_webrtc::RTCRtpTransceiver::wrap()->GetOrCreate(_factory, transceiver)->ToObject();
    MakeCallback("ontrack", 3, argv);
  }));
}

void node_webrtc::PeerConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) {
}

void node_webrtc::PeerConnection::OnRenegotiationNeeded() {
  Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>([this]() {
    Nan::HandleScope scope;
    MakeCallback("onnegotiationneeded", 0, nullptr);
  }));
}

NAN_METHOD(node_webrtc::PeerConnection::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the PeerConnection.");
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, node_webrtc::Maybe<node_webrtc::ExtendedRTCConfiguration>);

  // Tell em whats up
  auto obj = new node_webrtc::PeerConnection(configuration.FromMaybe(node_webrtc::ExtendedRTCConfiguration()));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(node_webrtc::PeerConnection::AddTrack) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError("Cannot addTrack; RTCPeerConnection is closed");
    return;
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<node_webrtc::MediaStreamTrack* COMMA node_webrtc::Maybe<MediaStream*>>);
  auto mediaStreamTrack = std::get<0>(pair);
  node_webrtc::Maybe<MediaStream*> mediaStream = std::get<1>(pair);
  std::vector<std::string> streams;
  if (mediaStream.IsJust()) {
    streams.push_back(mediaStream.UnsafeFromJust()->stream()->id());
  }
  auto result = self->_jinglePeerConnection->AddTrack(mediaStreamTrack->track(), streams);
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN(&result.error(), error, v8::Local<v8::Value>);
    Nan::ThrowError(error);
    return;
  }
  auto rtpSender = result.value();
  auto sender = node_webrtc::RTCRtpSender::wrap()->GetOrCreate(self->_factory, rtpSender);
  info.GetReturnValue().Set(sender->ToObject());
}

NAN_METHOD(node_webrtc::PeerConnection::AddTransceiver) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError("Cannot addTransceiver; RTCPeerConnection is closed");
    return;
  } else if (self->_jinglePeerConnection->GetConfiguration().sdp_semantics != webrtc::SdpSemantics::kUnifiedPlan) {
    Nan::ThrowError("AddTransceiver is only available with Unified Plan SdpSemanticsAbort");
    return;
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(args, std::tuple<node_webrtc::Either<cricket::MediaType COMMA node_webrtc::MediaStreamTrack*> COMMA node_webrtc::Maybe<webrtc::RtpTransceiverInit>>);
  node_webrtc::Either<cricket::MediaType, node_webrtc::MediaStreamTrack*> kindOrTrack = std::get<0>(args);
  node_webrtc::Maybe<webrtc::RtpTransceiverInit> maybeInit = std::get<1>(args);
  auto result = kindOrTrack.IsLeft()
      ? maybeInit.IsNothing()
      ? self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromLeft())
      : self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromLeft(), maybeInit.UnsafeFromJust())
      : maybeInit.IsNothing()
      ? self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromRight()->track())
      : self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromRight()->track(), maybeInit.UnsafeFromJust());
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN(&result.error(), error, v8::Local<v8::Value>);
    Nan::ThrowError(error);
    return;
  }
  auto rtpTransceiver = result.value();
  auto transceiver = node_webrtc::RTCRtpTransceiver::wrap()->GetOrCreate(self->_factory, rtpTransceiver);
  info.GetReturnValue().Set(transceiver->ToObject());
}

NAN_METHOD(node_webrtc::PeerConnection::RemoveTrack) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError(node_webrtc::ErrorFactory::CreateInvalidStateError("Cannot removeTrack; RTCPeerConnection is closed"));
    return;
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(sender, node_webrtc::RTCRtpSender*);
  auto senders = self->_jinglePeerConnection->GetSenders();
  if (std::find(senders.begin(), senders.end(), sender->sender()) == senders.end()) {
    Nan::ThrowError(node_webrtc::ErrorFactory::CreateInvalidAccessError("Cannot removeTrack"));
    return;
  }
  if (!self->_jinglePeerConnection->RemoveTrack(sender->sender())) {
    Nan::ThrowError(node_webrtc::ErrorFactory::CreateInvalidAccessError("Cannot removeTrack"));
    return;
  }
}

NAN_METHOD(node_webrtc::PeerConnection::CreateOffer) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver);

  auto maybeOptions = node_webrtc::From<node_webrtc::Maybe<RTCOfferOptions>>(node_webrtc::Arguments(info)).Map(
  [](const node_webrtc::Maybe<RTCOfferOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCOfferOptions()); });
  if (maybeOptions.IsInvalid()) {
    node_webrtc::Reject(resolver, node_webrtc::SomeError(maybeOptions.ToErrors()[0]));
    return;
  }

  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    node_webrtc::Reject(resolver, node_webrtc::ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'createOffer' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->CreateOffer(observer, maybeOptions.UnsafeFromValid().options);
}

NAN_METHOD(node_webrtc::PeerConnection::CreateAnswer) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver);

  auto maybeOptions = node_webrtc::From<node_webrtc::Maybe<RTCAnswerOptions>>(node_webrtc::Arguments(info)).Map(
  [](const node_webrtc::Maybe<RTCAnswerOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCAnswerOptions()); });
  if (maybeOptions.IsInvalid()) {
    node_webrtc::Reject(resolver, node_webrtc::SomeError(maybeOptions.ToErrors()[0]));
    return;
  }

  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    node_webrtc::Reject(resolver, node_webrtc::ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'createAnswer' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->CreateAnswer(observer, maybeOptions.UnsafeFromValid().options);
}

NAN_METHOD(node_webrtc::PeerConnection::SetLocalDescription) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver);

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, descriptionInit, node_webrtc::RTCSessionDescriptionInit);
  if (descriptionInit.sdp.empty()) {
    descriptionInit.sdp = self->_lastSdp.sdp;
  }

  CONVERT_OR_REJECT_AND_RETURN(resolver, descriptionInit, description, webrtc::SessionDescriptionInterface*);
  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    delete description;
    node_webrtc::Reject(resolver, node_webrtc::ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'setLocalDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->SetLocalDescription(observer, description);
}

NAN_METHOD(node_webrtc::PeerConnection::SetRemoteDescription) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver);

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, description, webrtc::SessionDescriptionInterface*);

  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    delete description;
    node_webrtc::Reject(resolver, node_webrtc::ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'setRemoteDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->SetRemoteDescription(observer, description);
}

NAN_METHOD(node_webrtc::PeerConnection::AddIceCandidate) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver);

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, rawCandidate, webrtc::IceCandidateInterface*);
  auto candidate = std::shared_ptr<webrtc::IceCandidateInterface>(rawCandidate);

  self->Dispatch(node_webrtc::CreatePromise<node_webrtc::PeerConnection>(resolver, [self, candidate](auto resolver) {
    if (self->_jinglePeerConnection
        && self->_jinglePeerConnection->signaling_state() != webrtc::PeerConnectionInterface::SignalingState::kClosed
        && self->_jinglePeerConnection->AddIceCandidate(candidate.get())) {
      node_webrtc::Resolve(resolver, Nan::Undefined());
    } else {
      std::string error = std::string("Failed to set ICE candidate");
      if (!self->_jinglePeerConnection
          || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
        error += "; RTCPeerConnection is closed";
      }
      error += ".";
      node_webrtc::Reject(resolver, node_webrtc::SomeError(error));
    }
  }));
}

NAN_METHOD(node_webrtc::PeerConnection::CreateDataChannel) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());

  if (self->_jinglePeerConnection == nullptr) {
    Nan::ThrowError(node_webrtc::ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'createDataChannel' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(args, std::tuple<std::string COMMA node_webrtc::Maybe<webrtc::DataChannelInit>>);

  auto label = std::get<0>(args);
  auto dataChannelInit = std::get<1>(args).FromMaybe(webrtc::DataChannelInit());

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface =
      self->_jinglePeerConnection->CreateDataChannel(label, &dataChannelInit);

  auto observer = new DataChannelObserver(self->_factory, data_channel_interface);
  auto channel = DataChannel::wrap()->GetOrCreate(observer, observer->channel());
  self->_channels.push_back(channel);

  info.GetReturnValue().Set(channel->ToObject());
}

NAN_METHOD(node_webrtc::PeerConnection::GetConfiguration) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());

  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? node_webrtc::ExtendedRTCConfiguration(self->_jinglePeerConnection->GetConfiguration(), self->_port_range)
      : self->_cached_configuration,
      configuration,
      v8::Local<v8::Value>);

  info.GetReturnValue().Set(configuration);
}

NAN_METHOD(node_webrtc::PeerConnection::SetConfiguration) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, webrtc::PeerConnectionInterface::RTCConfiguration);

  if (!self->_jinglePeerConnection) {
    Nan::ThrowError(node_webrtc::ErrorFactory::CreateInvalidStateError("RTCPeerConnection is closed"));
    return;
  }

  webrtc::RTCError rtcError;
  if (!self->_jinglePeerConnection->SetConfiguration(configuration, &rtcError)) {
    CONVERT_OR_THROW_AND_RETURN(&rtcError, error, v8::Local<v8::Value>);
    Nan::ThrowError(error);
    return;
  }
}

NAN_METHOD(node_webrtc::PeerConnection::GetReceivers) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  std::vector<node_webrtc::RTCRtpReceiver*> receivers;
  if (self->_jinglePeerConnection) {
    for (const auto& receiver : self->_jinglePeerConnection->GetReceivers()) {
      receivers.emplace_back(node_webrtc::RTCRtpReceiver::wrap()->GetOrCreate(self->_factory, receiver));
    }
  }
  CONVERT_OR_THROW_AND_RETURN(receivers, result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(node_webrtc::PeerConnection::GetSenders) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  std::vector<node_webrtc::RTCRtpSender*> senders;
  if (self->_jinglePeerConnection) {
    for (const auto& sender : self->_jinglePeerConnection->GetSenders()) {
      senders.emplace_back(node_webrtc::RTCRtpSender::wrap()->GetOrCreate(self->_factory, sender));
    }
  }
  CONVERT_OR_THROW_AND_RETURN(senders, result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(node_webrtc::PeerConnection::GetStats) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver);

  if (!self->_jinglePeerConnection) {
    node_webrtc::Reject(resolver, Nan::Error("RTCPeerConnection is closed"));
    return;
  }

  auto callback = new rtc::RefCountedObject<node_webrtc::RTCStatsCollector>(self, resolver);
  self->_jinglePeerConnection->GetStats(callback);
}

NAN_METHOD(node_webrtc::PeerConnection::LegacyGetStats) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver);

  if (!self->_jinglePeerConnection) {
    node_webrtc::Reject(resolver, Nan::Error("RTCPeerConnection is closed"));
    return;
  }

  auto statsObserver = new rtc::RefCountedObject<StatsObserver>(self, resolver);
  if (!self->_jinglePeerConnection->GetStats(statsObserver, nullptr,
          webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
    node_webrtc::Reject(resolver, Nan::Error("Failed to execute getStats"));
  }
}

NAN_METHOD(node_webrtc::PeerConnection::GetTransceivers) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());
  std::vector<node_webrtc::RTCRtpTransceiver*> transceivers;
  if (self->_jinglePeerConnection
      && self->_jinglePeerConnection->GetConfiguration().sdp_semantics == webrtc::SdpSemantics::kUnifiedPlan) {
    for (const auto& transceiver : self->_jinglePeerConnection->GetTransceivers()) {
      transceivers.emplace_back(node_webrtc::RTCRtpTransceiver::wrap()->GetOrCreate(self->_factory, transceiver));
    }
  }
  CONVERT_OR_THROW_AND_RETURN(transceivers, result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

NAN_METHOD(node_webrtc::PeerConnection::UpdateIce) {
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(node_webrtc::PeerConnection::Close) {
  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.This());

  if (self->_jinglePeerConnection) {
    self->_cached_configuration = node_webrtc::ExtendedRTCConfiguration(
            self->_jinglePeerConnection->GetConfiguration(),
            self->_port_range);
    self->_jinglePeerConnection->Close();
    // NOTE(mroberts): Perhaps another way to do this is to just register all remote MediaStreamTracks against this
    // RTCPeerConnection, not unlike what we do with RTCDataChannels.
    if (self->_jinglePeerConnection->GetConfiguration().sdp_semantics == webrtc::SdpSemantics::kUnifiedPlan) {
      for (const auto& transceiver : self->_jinglePeerConnection->GetTransceivers()) {
        auto track = node_webrtc::MediaStreamTrack::wrap()->GetOrCreate(self->_factory, transceiver->receiver()->track());
        track->OnPeerConnectionClosed();
      }
    }
    for (auto channel : self->_channels) {
      channel->OnPeerConnectionClosed();
    }
  }

  self->_jinglePeerConnection = nullptr;

  if (self->_factory) {
    if (self->_shouldReleaseFactory) {
      node_webrtc::PeerConnectionFactory::Release();
    }
    self->_factory = nullptr;
  }

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(node_webrtc::PeerConnection::GetCanTrickleIceCandidates) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(node_webrtc::PeerConnection::GetConnectionState) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());

  auto iceConnectionState = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed;

  CONVERT_OR_THROW_AND_RETURN(iceConnectionState, connectionState, node_webrtc::RTCPeerConnectionState);
  CONVERT_OR_THROW_AND_RETURN(connectionState, value, v8::Local<v8::Value>);

  info.GetReturnValue().Set(value);
}

NAN_GETTER(node_webrtc::PeerConnection::GetCurrentLocalDescription) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_local_description(), description, v8::Local<v8::Value>);
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::PeerConnection::GetLocalDescription) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->local_description(), description, v8::Local<v8::Value>);
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::PeerConnection::GetPendingLocalDescription) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_local_description(), description, v8::Local<v8::Value>);
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::PeerConnection::GetCurrentRemoteDescription) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_remote_description(), description, v8::Local<v8::Value>);
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::PeerConnection::GetRemoteDescription) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->remote_description(), description, v8::Local<v8::Value>);
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::PeerConnection::GetPendingRemoteDescription) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_remote_description(), description, v8::Local<v8::Value>);
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(node_webrtc::PeerConnection::GetSignalingState) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->signaling_state()
      : webrtc::PeerConnectionInterface::SignalingState::kClosed,
      state,
      v8::Local<v8::Value>);

  info.GetReturnValue().Set(state);
}

NAN_GETTER(node_webrtc::PeerConnection::GetIceConnectionState) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed,
      state,
      v8::Local<v8::Value>);

  info.GetReturnValue().Set(state);
}

NAN_GETTER(node_webrtc::PeerConnection::GetIceGatheringState) {
  (void) property;

  auto self = node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::PeerConnection>::Unwrap(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_gathering_state()
      : webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete,
      state,
      v8::Local<v8::Value>);

  info.GetReturnValue().Set(state);
}

void node_webrtc::PeerConnection::SaveLastSdp(const node_webrtc::RTCSessionDescriptionInit& lastSdp) {
  this->_lastSdp = lastSdp;
}

void node_webrtc::PeerConnection::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnection").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "addTrack", AddTrack);
  Nan::SetPrototypeMethod(tpl, "addTransceiver", AddTransceiver);
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
  Nan::SetPrototypeMethod(tpl, "legacyGetStats", LegacyGetStats);
  Nan::SetPrototypeMethod(tpl, "getTransceivers", GetTransceivers);
  Nan::SetPrototypeMethod(tpl, "updateIce", UpdateIce);
  Nan::SetPrototypeMethod(tpl, "addIceCandidate", AddIceCandidate);
  Nan::SetPrototypeMethod(tpl, "createDataChannel", CreateDataChannel);
  Nan::SetPrototypeMethod(tpl, "close", Close);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("canTrickleIceCandidates").ToLocalChecked(), GetCanTrickleIceCandidates, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("connectionState").ToLocalChecked(), GetConnectionState, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentLocalDescription").ToLocalChecked(), GetCurrentLocalDescription, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("localDescription").ToLocalChecked(), GetLocalDescription, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingLocalDescription").ToLocalChecked(), GetPendingLocalDescription, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentRemoteDescription").ToLocalChecked(), GetCurrentRemoteDescription, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remoteDescription").ToLocalChecked(), GetRemoteDescription, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingRemoteDescription").ToLocalChecked(), GetPendingRemoteDescription, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("signalingState").ToLocalChecked(), GetSignalingState, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceConnectionState").ToLocalChecked(), GetIceConnectionState, nullptr);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceGatheringState").ToLocalChecked(), GetIceGatheringState, nullptr);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("PeerConnection").ToLocalChecked(), tpl->GetFunction());
}
