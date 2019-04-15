/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_peer_connection.h"

#include <iosfwd>

#include <webrtc/api/media_types.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/rtc_error.h>
#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/p2p/client/basic_port_allocator.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/interfaces.h"
#include "src/converters/v8.h"
#include "src/dictionaries/node_webrtc/rtc_answer_options.h"
#include "src/dictionaries/node_webrtc/rtc_offer_options.h"
#include "src/dictionaries/node_webrtc/some_error.h"
#include "src/dictionaries/webrtc/data_channel_init.h"
#include "src/dictionaries/webrtc/ice_candidate_interface.h"
#include "src/dictionaries/webrtc/rtc_configuration.h"
#include "src/dictionaries/webrtc/rtc_error.h"
#include "src/dictionaries/webrtc/rtp_transceiver_init.h"
#include "src/enums/node_webrtc/rtc_peer_connection_state.h"
#include "src/enums/webrtc/ice_connection_state.h"
#include "src/enums/webrtc/ice_gathering_state.h"
#include "src/enums/webrtc/media_type.h"
#include "src/enums/webrtc/signaling_state.h"
#include "src/functional/either.h"
#include "src/functional/maybe.h"
#include "src/interfaces/media_stream.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_data_channel.h"
#include "src/interfaces/rtc_peer_connection/create_session_description_observer.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"
#include "src/interfaces/rtc_peer_connection/rtc_stats_collector.h"
#include "src/interfaces/rtc_peer_connection/set_session_description_observer.h"
#include "src/interfaces/rtc_peer_connection/stats_observer.h"
#include "src/interfaces/rtc_rtp_receiver.h"
#include "src/interfaces/rtc_rtp_sender.h"
#include "src/interfaces/rtc_rtp_transceiver.h"
#include "src/node/async_object_wrap_with_loop.h"
#include "src/node/error_factory.h"
#include "src/node/events.h"
#include "src/node/promise.h"
#include "src/node/utility.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCPeerConnection::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

//
// PeerConnection
//

RTCPeerConnection::RTCPeerConnection(const ExtendedRTCConfiguration& configuration)
  : AsyncObjectWrapWithLoop<RTCPeerConnection>("RTCPeerConnection", *this) {

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

RTCPeerConnection::~RTCPeerConnection() {
  _jinglePeerConnection = nullptr;
  _channels.clear();
  if (_factory) {
    if (_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }
}

void RTCPeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState state) {
  Dispatch(CreateCallback<RTCPeerConnection>([this, state]() {
    Nan::HandleScope scope;
    MakeCallback("onsignalingstatechange", 0, nullptr);
    if (state == webrtc::PeerConnectionInterface::kClosed) {
      Stop();
    }
  }));
}

void RTCPeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState) {
  Dispatch(CreateCallback<RTCPeerConnection>([this]() {
    Nan::HandleScope scope;
    MakeCallback("oniceconnectionstatechange", 0, nullptr);
    MakeCallback("onconnectionstatechange", 0, nullptr);
  }));
}

void RTCPeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState) {
  Dispatch(CreateCallback<RTCPeerConnection>([this]() {
    Nan::HandleScope scope;
    MakeCallback("onicegatheringstatechange", 0, nullptr);
  }));
}

void RTCPeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* ice_candidate) {
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

  Dispatch(CreateCallback<RTCPeerConnection>([this, candidate, error]() {
    Nan::HandleScope scope;
    if (error.empty()) {
      auto maybeCandidate = From<v8::Local<v8::Value>>(candidate.get());
      if (maybeCandidate.IsValid()) {
        v8::Local<v8::Value> argv[1];
        argv[0] = maybeCandidate.UnsafeFromValid();
        MakeCallback("onicecandidate", 1, argv);
      }
    }
  }));
}

void RTCPeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
  auto observer = new DataChannelObserver(_factory, channel);
  Dispatch(CreateCallback<RTCPeerConnection>([this, observer]() {
    Nan::HandleScope scope;
    auto channel = RTCDataChannel::wrap()->GetOrCreate(observer, observer->channel());
    v8::Local<v8::Value> argv = napi::UnsafeToV8(channel->Value());
    MakeCallback("ondatachannel", 1, &argv);
  }));
}

void RTCPeerConnection::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) {
}

void RTCPeerConnection::OnAddTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>& streams) {
  if (_jinglePeerConnection->GetConfiguration().sdp_semantics != webrtc::SdpSemantics::kPlanB) {
    return;
  }
  Dispatch(CreateCallback<RTCPeerConnection>([this, receiver, streams]() {
    Nan::HandleScope scope;

    auto mediaStreams = std::vector<MediaStream*>();
    for (auto const& stream : streams) {
      auto mediaStream = MediaStream::wrap()->GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN(mediaStreams, streamArray, v8::Local<v8::Value>)

    v8::Local<v8::Value> argv[3];
    argv[0] = napi::UnsafeToV8(RTCRtpReceiver::wrap()->GetOrCreate(_factory, receiver)->Value());
    argv[1] = streamArray;
    argv[2] = Nan::Null();
    MakeCallback("ontrack", 3, argv);
  }));
}

void RTCPeerConnection::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  auto receiver = transceiver->receiver();
  auto streams = receiver->streams();
  Dispatch(CreateCallback<RTCPeerConnection>([this, transceiver, receiver, streams]() {
    Nan::HandleScope scope;

    auto mediaStreams = std::vector<MediaStream*>();
    for (auto const& stream : streams) {
      auto mediaStream = MediaStream::wrap()->GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN(mediaStreams, streamArray, v8::Local<v8::Value>)

    v8::Local<v8::Value> argv[3];
    argv[0] = napi::UnsafeToV8(RTCRtpReceiver::wrap()->GetOrCreate(_factory, receiver)->Value());
    argv[1] = streamArray;
    argv[2] = napi::UnsafeToV8(RTCRtpTransceiver::wrap()->GetOrCreate(_factory, transceiver)->Value());
    MakeCallback("ontrack", 3, argv);
  }));
}

void RTCPeerConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) {
}

void RTCPeerConnection::OnRenegotiationNeeded() {
  Dispatch(CreateCallback<RTCPeerConnection>([this]() {
    Nan::HandleScope scope;
    MakeCallback("onnegotiationneeded", 0, nullptr);
  }));
}

NAN_METHOD(RTCPeerConnection::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the RTCPeerConnection.");
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, Maybe<ExtendedRTCConfiguration>)

  // Tell em whats up
  auto obj = new RTCPeerConnection(configuration.FromMaybe(ExtendedRTCConfiguration()));
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(RTCPeerConnection::AddTrack) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError("Cannot addTrack; RTCPeerConnection is closed");
    return;
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<MediaStreamTrack* COMMA Maybe<MediaStream*>>)
  auto mediaStreamTrack = std::get<0>(pair);
  Maybe<MediaStream*> mediaStream = std::get<1>(pair);
  std::vector<std::string> streams;
  if (mediaStream.IsJust()) {
    streams.push_back(mediaStream.UnsafeFromJust()->stream()->id());
  }
  auto result = self->_jinglePeerConnection->AddTrack(mediaStreamTrack->track(), streams);
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN(&result.error(), error, v8::Local<v8::Value>)
    Nan::ThrowError(error);
    return;
  }
  auto rtpSender = result.value();
  auto sender = RTCRtpSender::wrap()->GetOrCreate(self->_factory, rtpSender);
  info.GetReturnValue().Set(napi::UnsafeToV8(sender->Value()));
}

NAN_METHOD(RTCPeerConnection::AddTransceiver) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError("Cannot addTransceiver; RTCPeerConnection is closed");
    return;
  } else if (self->_jinglePeerConnection->GetConfiguration().sdp_semantics != webrtc::SdpSemantics::kUnifiedPlan) {
    Nan::ThrowError("AddTransceiver is only available with Unified Plan SdpSemanticsAbort");
    return;
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(args, std::tuple<Either<cricket::MediaType COMMA MediaStreamTrack*> COMMA Maybe<webrtc::RtpTransceiverInit>>)
  Either<cricket::MediaType, MediaStreamTrack*> kindOrTrack = std::get<0>(args);
  Maybe<webrtc::RtpTransceiverInit> maybeInit = std::get<1>(args);
  auto result = kindOrTrack.IsLeft()
      ? maybeInit.IsNothing()
      ? self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromLeft())
      : self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromLeft(), maybeInit.UnsafeFromJust())
      : maybeInit.IsNothing()
      ? self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromRight()->track())
      : self->_jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromRight()->track(), maybeInit.UnsafeFromJust());
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN(&result.error(), error, v8::Local<v8::Value>)
    Nan::ThrowError(error);
    return;
  }
  auto rtpTransceiver = result.value();
  auto transceiver = RTCRtpTransceiver::wrap()->GetOrCreate(self->_factory, rtpTransceiver);
  info.GetReturnValue().Set(napi::UnsafeToV8(transceiver->Value()));
}

NAN_METHOD(RTCPeerConnection::RemoveTrack) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  if (!self->_jinglePeerConnection) {
    Nan::ThrowError(ErrorFactory::CreateInvalidStateError("Cannot removeTrack; RTCPeerConnection is closed"));
    return;
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN(sender, RTCRtpSender*)
  auto senders = self->_jinglePeerConnection->GetSenders();
  if (std::find(senders.begin(), senders.end(), sender->sender()) == senders.end()) {
    Nan::ThrowError(ErrorFactory::CreateInvalidAccessError("Cannot removeTrack"));
    return;
  }
  if (!self->_jinglePeerConnection->RemoveTrack(sender->sender())) {
    Nan::ThrowError(ErrorFactory::CreateInvalidAccessError("Cannot removeTrack"));
    return;
  }
}

NAN_METHOD(RTCPeerConnection::CreateOffer) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver)

  auto maybeOptions = From<Maybe<RTCOfferOptions>>(Arguments(info)).Map([](auto maybeOptions) {
    return maybeOptions.FromMaybe(RTCOfferOptions());
  });
  if (maybeOptions.IsInvalid()) {
    Reject(resolver, SomeError(maybeOptions.ToErrors()[0]));
    return;
  }

  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(resolver, ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'createOffer' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->CreateOffer(observer, maybeOptions.UnsafeFromValid().options);
}

NAN_METHOD(RTCPeerConnection::CreateAnswer) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver)

  auto maybeOptions = From<Maybe<RTCAnswerOptions>>(Arguments(info)).Map([](auto maybeOptions) {
    return maybeOptions.FromMaybe(RTCAnswerOptions());
  });
  if (maybeOptions.IsInvalid()) {
    Reject(resolver, SomeError(maybeOptions.ToErrors()[0]));
    return;
  }

  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(resolver, ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'createAnswer' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->CreateAnswer(observer, maybeOptions.UnsafeFromValid().options);
}

NAN_METHOD(RTCPeerConnection::SetLocalDescription) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver)

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, descriptionInit, RTCSessionDescriptionInit)
  if (descriptionInit.sdp.empty()) {
    descriptionInit.sdp = self->_lastSdp.sdp;
  }

  CONVERT_OR_REJECT_AND_RETURN(resolver, descriptionInit, rawDescription, webrtc::SessionDescriptionInterface*)
  std::unique_ptr<webrtc::SessionDescriptionInterface> description(rawDescription);

  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(resolver, ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'setLocalDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->SetLocalDescription(observer, description.release());
}

NAN_METHOD(RTCPeerConnection::SetRemoteDescription) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver)

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, rawDescription, webrtc::SessionDescriptionInterface*)
  std::unique_ptr<webrtc::SessionDescriptionInterface> description(rawDescription);

  if (!self->_jinglePeerConnection || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(resolver, ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'setRemoteDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(self, resolver);
  self->_jinglePeerConnection->SetRemoteDescription(observer, description.release());
}  // NOLINT

NAN_METHOD(RTCPeerConnection::AddIceCandidate) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver)

  CONVERT_ARGS_OR_REJECT_AND_RETURN(resolver, candidate, std::shared_ptr<webrtc::IceCandidateInterface>)

  self->Dispatch(CreatePromise<RTCPeerConnection>(resolver, [self, candidate](auto resolver) {
    if (self->_jinglePeerConnection
        && self->_jinglePeerConnection->signaling_state() != webrtc::PeerConnectionInterface::SignalingState::kClosed
        && self->_jinglePeerConnection->AddIceCandidate(candidate.get())) {
      Resolve(resolver, Nan::Undefined());
    } else {
      std::string error = std::string("Failed to set ICE candidate");
      if (!self->_jinglePeerConnection
          || self->_jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
        error += "; RTCPeerConnection is closed";
      }
      error += ".";
      Reject(resolver, SomeError(error));
    }
  }));
}

NAN_METHOD(RTCPeerConnection::CreateDataChannel) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());

  if (self->_jinglePeerConnection == nullptr) {
    Nan::ThrowError(ErrorFactory::CreateInvalidStateError(
            "Failed to execute 'createDataChannel' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return;
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(args, std::tuple<std::string COMMA Maybe<webrtc::DataChannelInit>>)

  auto label = std::get<0>(args);
  auto dataChannelInit = std::get<1>(args).FromMaybe(webrtc::DataChannelInit());

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface =
      self->_jinglePeerConnection->CreateDataChannel(label, &dataChannelInit);

  if (!data_channel_interface) {
    Nan::ThrowError(ErrorFactory::CreateInvalidStateError("'createDataChannel' failed"));
    return;
  }

  auto observer = new DataChannelObserver(self->_factory, data_channel_interface);
  auto channel = RTCDataChannel::wrap()->GetOrCreate(observer, observer->channel());
  self->_channels.push_back(channel);

  info.GetReturnValue().Set(napi::UnsafeToV8(channel->Value()));
}

NAN_METHOD(RTCPeerConnection::GetConfiguration) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());

  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? ExtendedRTCConfiguration(self->_jinglePeerConnection->GetConfiguration(), self->_port_range)
      : self->_cached_configuration,
      configuration,
      v8::Local<v8::Value>)

  info.GetReturnValue().Set(configuration);
}

NAN_METHOD(RTCPeerConnection::SetConfiguration) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, webrtc::PeerConnectionInterface::RTCConfiguration)

  if (!self->_jinglePeerConnection) {
    Nan::ThrowError(ErrorFactory::CreateInvalidStateError("RTCPeerConnection is closed"));
    return;
  }

  webrtc::RTCError rtcError;
  if (!self->_jinglePeerConnection->SetConfiguration(configuration, &rtcError)) {
    CONVERT_OR_THROW_AND_RETURN(&rtcError, error, v8::Local<v8::Value>)
    Nan::ThrowError(error);
    return;
  }
}

NAN_METHOD(RTCPeerConnection::GetReceivers) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  std::vector<RTCRtpReceiver*> receivers;
  if (self->_jinglePeerConnection) {
    for (const auto& receiver : self->_jinglePeerConnection->GetReceivers()) {
      receivers.emplace_back(RTCRtpReceiver::wrap()->GetOrCreate(self->_factory, receiver));
    }
  }
  CONVERT_OR_THROW_AND_RETURN(receivers, result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCPeerConnection::GetSenders) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  std::vector<RTCRtpSender*> senders;
  if (self->_jinglePeerConnection) {
    for (const auto& sender : self->_jinglePeerConnection->GetSenders()) {
      senders.emplace_back(RTCRtpSender::wrap()->GetOrCreate(self->_factory, sender));
    }
  }
  CONVERT_OR_THROW_AND_RETURN(senders, result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCPeerConnection::GetStats) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver)

  if (!self->_jinglePeerConnection) {
    Reject(resolver, Nan::Error("RTCPeerConnection is closed"));
    return;
  }

  auto callback = new rtc::RefCountedObject<RTCStatsCollector>(self, resolver);
  self->_jinglePeerConnection->GetStats(callback);
}  // NOLINT

NAN_METHOD(RTCPeerConnection::LegacyGetStats) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  RETURNS_PROMISE(resolver)

  if (!self->_jinglePeerConnection) {
    Reject(resolver, Nan::Error("RTCPeerConnection is closed"));
    return;
  }

  auto statsObserver = new rtc::RefCountedObject<StatsObserver>(self, resolver);
  if (!self->_jinglePeerConnection->GetStats(statsObserver, nullptr,
          webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
    Reject(resolver, Nan::Error("Failed to execute getStats"));
  }
}

NAN_METHOD(RTCPeerConnection::GetTransceivers) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());
  std::vector<RTCRtpTransceiver*> transceivers;
  if (self->_jinglePeerConnection
      && self->_jinglePeerConnection->GetConfiguration().sdp_semantics == webrtc::SdpSemantics::kUnifiedPlan) {
    for (const auto& transceiver : self->_jinglePeerConnection->GetTransceivers()) {
      transceivers.emplace_back(RTCRtpTransceiver::wrap()->GetOrCreate(self->_factory, transceiver));
    }
  }
  CONVERT_OR_THROW_AND_RETURN(transceivers, result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
}

NAN_METHOD(RTCPeerConnection::UpdateIce) {
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(RTCPeerConnection::Close) {
  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.This());

  if (self->_jinglePeerConnection) {
    self->_cached_configuration = ExtendedRTCConfiguration(
            self->_jinglePeerConnection->GetConfiguration(),
            self->_port_range);
    self->_jinglePeerConnection->Close();
    // NOTE(mroberts): Perhaps another way to do this is to just register all remote MediaStreamTracks against this
    // RTCPeerConnection, not unlike what we do with RTCDataChannels.
    if (self->_jinglePeerConnection->GetConfiguration().sdp_semantics == webrtc::SdpSemantics::kUnifiedPlan) {
      for (const auto& transceiver : self->_jinglePeerConnection->GetTransceivers()) {
        auto track = MediaStreamTrack::wrap()->GetOrCreate(self->_factory, transceiver->receiver()->track());
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
      PeerConnectionFactory::Release();
    }
    self->_factory = nullptr;
  }

  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(RTCPeerConnection::GetCanTrickleIceCandidates) {
  (void) property;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(RTCPeerConnection::GetConnectionState) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());

  auto iceConnectionState = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed;

  CONVERT_OR_THROW_AND_RETURN(iceConnectionState, connectionState, RTCPeerConnectionState)
  CONVERT_OR_THROW_AND_RETURN(connectionState, value, v8::Local<v8::Value>)

  info.GetReturnValue().Set(value);
}

NAN_GETTER(RTCPeerConnection::GetCurrentLocalDescription) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_local_description(), description, v8::Local<v8::Value>)
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCPeerConnection::GetLocalDescription) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->local_description(), description, v8::Local<v8::Value>)
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCPeerConnection::GetPendingLocalDescription) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_local_description(), description, v8::Local<v8::Value>)
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCPeerConnection::GetCurrentRemoteDescription) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_remote_description(), description, v8::Local<v8::Value>)
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCPeerConnection::GetRemoteDescription) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->remote_description(), description, v8::Local<v8::Value>)
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCPeerConnection::GetPendingRemoteDescription) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());

  v8::Local<v8::Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_remote_description(), description, v8::Local<v8::Value>)
    result = description;
  }

  info.GetReturnValue().Set(result);
}

NAN_GETTER(RTCPeerConnection::GetSignalingState) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->signaling_state()
      : webrtc::PeerConnectionInterface::SignalingState::kClosed,
      state,
      v8::Local<v8::Value>)

  info.GetReturnValue().Set(state);
}

NAN_GETTER(RTCPeerConnection::GetIceConnectionState) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed,
      state,
      v8::Local<v8::Value>)

  info.GetReturnValue().Set(state);
}

NAN_GETTER(RTCPeerConnection::GetIceGatheringState) {
  (void) property;

  auto self = AsyncObjectWrapWithLoop<RTCPeerConnection>::Unwrap(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_gathering_state()
      : webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete,
      state,
      v8::Local<v8::Value>)

  info.GetReturnValue().Set(state);
}

void RTCPeerConnection::SaveLastSdp(const RTCSessionDescriptionInit& lastSdp) {
  this->_lastSdp = lastSdp;
}

void RTCPeerConnection::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("RTCPeerConnection").ToLocalChecked());
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
  exports->Set(Nan::New("RTCPeerConnection").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace node_webrtc
