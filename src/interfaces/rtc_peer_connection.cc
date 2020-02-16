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
#include "src/converters/napi.h"
#include "src/dictionaries/macros/napi.h"
#include "src/dictionaries/node_webrtc/rtc_answer_options.h"
#include "src/dictionaries/node_webrtc/rtc_offer_options.h"
#include "src/dictionaries/node_webrtc/rtc_session_description_init.h"
#include "src/dictionaries/node_webrtc/some_error.h"
#include "src/dictionaries/webrtc/data_channel_init.h"
#include "src/dictionaries/webrtc/ice_candidate_interface.h"
#include "src/dictionaries/webrtc/rtc_configuration.h"
#include "src/dictionaries/webrtc/rtc_error.h"
#include "src/dictionaries/webrtc/rtp_transceiver_init.h"
#include "src/enums/webrtc/ice_connection_state.h"
#include "src/enums/webrtc/ice_gathering_state.h"
#include "src/enums/webrtc/media_type.h"
#include "src/enums/webrtc/peer_connection_state.h"
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
#include "src/interfaces/rtc_sctp_transport.h"
#include "src/node/error_factory.h"
#include "src/node/events.h"
#include "src/node/promise.h"
#include "src/node/utility.h"

namespace node_webrtc {

Napi::FunctionReference& RTCPeerConnection::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

//
// PeerConnection
//

RTCPeerConnection::RTCPeerConnection(const Napi::CallbackInfo& info)
  : AsyncObjectWrapWithLoop<RTCPeerConnection>("RTCPeerConnection", *this, info) {
  auto env = info.Env();

  if (!info.IsConstructCall()) {
    Napi::TypeError::New(env, "Use the new operator to construct the RTCPeerConnection.").ThrowAsJavaScriptException();
    return;
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN_VOID_NAPI(info, maybeConfiguration, Maybe<ExtendedRTCConfiguration>)

  auto configuration = maybeConfiguration.FromMaybe(ExtendedRTCConfiguration());

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
    MakeCallback("onsignalingstatechange", {});
    if (state == webrtc::PeerConnectionInterface::kClosed) {
      Stop();
    }
  }));
}

void RTCPeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState) {
  Dispatch(CreateCallback<RTCPeerConnection>([this]() {
    MakeCallback("oniceconnectionstatechange", {});
    MakeCallback("onconnectionstatechange", {});
  }));
}

void RTCPeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState) {
  Dispatch(CreateCallback<RTCPeerConnection>([this]() {
    MakeCallback("onicegatheringstatechange", {});
  }));
}

void RTCPeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* ice_candidate) {
  std::string error;

  std::string sdp;
  if (!ice_candidate->ToString(&sdp)) {
    error = "Failed to print the candidate string. This is pretty weird. File a bug on https://github.com/node-webrtc/node-webrtc";
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
    if (error.empty()) {
      auto env = Env();
      auto maybeCandidate = From<Napi::Value>(std::make_pair(env, candidate.get()));
      if (maybeCandidate.IsValid()) {
        MakeCallback("onicecandidate", { maybeCandidate.UnsafeFromValid() });
      }
    }
  }));
}

static Validation<Napi::Value> CreateRTCPeerConnectionIceErrorEvent(
    const Napi::Value hostCandidate,
    const Napi::Value url,
    const Napi::Value errorCode,
    const Napi::Value errorText) {
  auto env = hostCandidate.Env();
  Napi::EscapableHandleScope scope(env);
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "hostCandidate", hostCandidate)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "url", url)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "errorCode", errorCode)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "errorText", errorText)
  return Pure(scope.Escape(object));
}

void RTCPeerConnection::OnIceCandidateError(const std::string& host_candidate, const std::string& url, int error_code, const std::string& error_text) {
  Dispatch(CreateCallback<RTCPeerConnection>([this, host_candidate, url, error_code, error_text]() {
    auto env = Env();
    auto maybeEvent = Validation<Napi::Value>::Join(curry(CreateRTCPeerConnectionIceErrorEvent)
            % From<Napi::Value>(std::make_pair(env, host_candidate))
            * From<Napi::Value>(std::make_pair(env, url))
            * From<Napi::Value>(std::make_pair(env, error_code))
            * From<Napi::Value>(std::make_pair(env, error_text)));
    if (maybeEvent.IsValid()) {
      MakeCallback("onicecandidateerror", { maybeEvent.UnsafeFromValid() });
    }
  }));
}

void RTCPeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
  auto observer = new DataChannelObserver(_factory, channel);
  Dispatch(CreateCallback<RTCPeerConnection>([this, observer]() {
    auto channel = RTCDataChannel::wrap()->GetOrCreate(observer, observer->channel());
    MakeCallback("ondatachannel", { channel->Value() });
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
    auto mediaStreams = std::vector<MediaStream*>();
    for (auto const& stream : streams) {
      auto mediaStream = MediaStream::wrap()->GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN_VOID_NAPI(Env(), mediaStreams, streamArray, Napi::Value)
    MakeCallback("ontrack", {
      RTCRtpReceiver::wrap()->GetOrCreate(_factory, receiver)->Value(),
      streamArray,
      Env().Null()
    });
  }));
}

void RTCPeerConnection::OnTrack(rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  auto receiver = transceiver->receiver();
  auto streams = receiver->streams();
  Dispatch(CreateCallback<RTCPeerConnection>([this, transceiver, receiver, streams]() {
    auto mediaStreams = std::vector<MediaStream*>();
    for (auto const& stream : streams) {
      auto mediaStream = MediaStream::wrap()->GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN_VOID_NAPI(Env(), mediaStreams, streamArray, Napi::Value)
    MakeCallback("ontrack", {
      RTCRtpReceiver::wrap()->GetOrCreate(_factory, receiver)->Value(),
      streamArray,
      RTCRtpTransceiver::wrap()->GetOrCreate(_factory, transceiver)->Value()
    });
  }));
}

void RTCPeerConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) {
}

void RTCPeerConnection::OnRenegotiationNeeded() {
  Dispatch(CreateCallback<RTCPeerConnection>([this]() {
    MakeCallback("onnegotiationneeded", {});
  }));
}

Napi::Value RTCPeerConnection::AddTrack(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(env, "Cannot addTrack; RTCPeerConnection is closed")).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, pair, std::tuple<MediaStreamTrack* COMMA Maybe<std::vector<MediaStream*>>>)
  auto mediaStreamTrack = std::get<0>(pair);
  Maybe<std::vector<MediaStream*>> mediaStreams = std::get<1>(pair);
  std::vector<std::string> streamIds;
  if (mediaStreams.IsJust()) {
    streamIds.reserve(mediaStreams.UnsafeFromJust().size());
    for (auto const& stream : mediaStreams.UnsafeFromJust()) {
      streamIds.emplace_back(stream->stream()->id());
    }
  }
  auto result = _jinglePeerConnection->AddTrack(mediaStreamTrack->track(), streamIds);
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, &result.error(), error, Napi::Value)
    Napi::Error(env, error).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  auto rtpSender = result.value();
  return RTCRtpSender::wrap()->GetOrCreate(_factory, rtpSender)->Value();
}

Napi::Value RTCPeerConnection::AddTransceiver(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    Napi::Error::New(env, "Cannot addTransceiver; RTCPeerConnection is closed").ThrowAsJavaScriptException();
    return env.Undefined();
  } else if (_jinglePeerConnection->GetConfiguration().sdp_semantics != webrtc::SdpSemantics::kUnifiedPlan) {
    Napi::Error::New(env, "AddTransceiver is only available with Unified Plan SdpSemanticsAbort").ThrowAsJavaScriptException();
    return env.Undefined();
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, args, std::tuple<Either<cricket::MediaType COMMA MediaStreamTrack*> COMMA Maybe<webrtc::RtpTransceiverInit>>)
  Either<cricket::MediaType, MediaStreamTrack*> kindOrTrack = std::get<0>(args);
  Maybe<webrtc::RtpTransceiverInit> maybeInit = std::get<1>(args);
  auto result = kindOrTrack.IsLeft()
      ? maybeInit.IsNothing()
      ? _jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromLeft())
      : _jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromLeft(), maybeInit.UnsafeFromJust())
      : maybeInit.IsNothing()
      ? _jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromRight()->track())
      : _jinglePeerConnection->AddTransceiver(kindOrTrack.UnsafeFromRight()->track(), maybeInit.UnsafeFromJust());
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, &result.error(), error, Napi::Value)
    Napi::Error(env, error).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  auto rtpTransceiver = result.value();
  return RTCRtpTransceiver::wrap()->GetOrCreate(_factory, rtpTransceiver)->Value();
}

Napi::Value RTCPeerConnection::RemoveTrack(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(env, "Cannot removeTrack; RTCPeerConnection is closed")).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, sender, RTCRtpSender*)
  auto senders = _jinglePeerConnection->GetSenders();
  if (std::find(senders.begin(), senders.end(), sender->sender()) == senders.end()) {
    Napi::Error(env, ErrorFactory::CreateInvalidAccessError(env, "Cannot removeTrack")).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  if (!_jinglePeerConnection->RemoveTrack(sender->sender())) {
    Napi::Error(env, ErrorFactory::CreateInvalidAccessError(env, "Cannot removeTrack")).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  return env.Undefined();
}

Napi::Value RTCPeerConnection::CreateOffer(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  auto maybeOptions = From<Maybe<RTCOfferOptions>>(Arguments(info)).Map([](auto maybeOptions) {
    return maybeOptions.FromMaybe(RTCOfferOptions());
  });
  if (maybeOptions.IsInvalid()) {
    Reject(deferred, SomeError(maybeOptions.ToErrors()[0]));
    return deferred.Promise();
  }

  if (!_jinglePeerConnection || _jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(deferred, ErrorFactory::CreateInvalidStateError(env,
            "Failed to execute 'createOffer' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(this, deferred);
  _jinglePeerConnection->CreateOffer(observer, maybeOptions.UnsafeFromValid().options);

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::CreateAnswer(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  auto maybeOptions = From<Maybe<RTCAnswerOptions>>(Arguments(info)).Map([](auto maybeOptions) {
    return maybeOptions.FromMaybe(RTCAnswerOptions());
  });
  if (maybeOptions.IsInvalid()) {
    Reject(deferred, SomeError(maybeOptions.ToErrors()[0]));
    return deferred.Promise();
  }

  if (!_jinglePeerConnection || _jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(deferred, ErrorFactory::CreateInvalidStateError(env,
            "Failed to execute 'createAnswer' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(this, deferred);
  _jinglePeerConnection->CreateAnswer(observer, maybeOptions.UnsafeFromValid().options);

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::SetLocalDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, descriptionInit, RTCSessionDescriptionInit)
  if (descriptionInit.sdp.empty()) {
    descriptionInit.sdp = _lastSdp.sdp;
  }

  auto maybeRawDescription = From<webrtc::SessionDescriptionInterface*>(descriptionInit);
  if (maybeRawDescription.IsInvalid()) {
    Reject(deferred, maybeRawDescription.ToErrors()[0]);
    return deferred.Promise();
  }
  auto rawDescription = maybeRawDescription.UnsafeFromValid();
  std::unique_ptr<webrtc::SessionDescriptionInterface> description(rawDescription);

  if (!_jinglePeerConnection || _jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(deferred, ErrorFactory::CreateInvalidStateError(env,
            "Failed to execute 'setLocalDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(this, deferred);
  _jinglePeerConnection->SetLocalDescription(observer, description.release());

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::SetRemoteDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, rawDescription, webrtc::SessionDescriptionInterface*)
  std::unique_ptr<webrtc::SessionDescriptionInterface> description(rawDescription);

  if (!_jinglePeerConnection || _jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(deferred, ErrorFactory::CreateInvalidStateError(env,
            "Failed to execute 'setRemoteDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto observer = new rtc::RefCountedObject<SetSessionDescriptionObserver>(this, deferred);
  _jinglePeerConnection->SetRemoteDescription(observer, description.release());

  return deferred.Promise();  // NOLINT
}

Napi::Value RTCPeerConnection::AddIceCandidate(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, candidate, std::shared_ptr<webrtc::IceCandidateInterface>)

  Dispatch(CreatePromise<RTCPeerConnection>(deferred, [this, candidate](auto deferred) {
    if (_jinglePeerConnection
        && _jinglePeerConnection->signaling_state() != webrtc::PeerConnectionInterface::SignalingState::kClosed
        && _jinglePeerConnection->AddIceCandidate(candidate.get())) {
      Resolve(deferred, this->Env().Undefined());
    } else {
      std::string error = std::string("Failed to set ICE candidate");
      if (!_jinglePeerConnection
          || _jinglePeerConnection->signaling_state() == webrtc::PeerConnectionInterface::SignalingState::kClosed) {
        error += "; RTCPeerConnection is closed";
      }
      error += ".";
      Reject(deferred, SomeError(error));
    }
  }));

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::CreateDataChannel(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (_jinglePeerConnection == nullptr) {
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(env,
            "Failed to execute 'createDataChannel' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'.")).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, args, std::tuple<std::string COMMA Maybe<webrtc::DataChannelInit>>)

  auto label = std::get<0>(args);
  auto dataChannelInit = std::get<1>(args).FromMaybe(webrtc::DataChannelInit());

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface =
      _jinglePeerConnection->CreateDataChannel(label, &dataChannelInit);

  if (!data_channel_interface) {
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(env, "'createDataChannel' failed")).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  auto observer = new DataChannelObserver(_factory, data_channel_interface);
  auto channel = RTCDataChannel::wrap()->GetOrCreate(observer, observer->channel());
  _channels.push_back(channel);

  return channel->Value();
}

Napi::Value RTCPeerConnection::GetConfiguration(const Napi::CallbackInfo& info) {
  auto configuration = _jinglePeerConnection
      ? ExtendedRTCConfiguration(_jinglePeerConnection->GetConfiguration(), _port_range)
      : _cached_configuration;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), configuration, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::SetConfiguration(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, configuration, webrtc::PeerConnectionInterface::RTCConfiguration)

  if (!_jinglePeerConnection) {
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(env, "RTCPeerConnection is closed")).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  auto rtcError = _jinglePeerConnection->SetConfiguration(configuration);
  if (!rtcError.ok()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, &rtcError, error, Napi::Value)
    Napi::Error(env, error).ThrowAsJavaScriptException();
    return env.Undefined();
  }

  return env.Undefined();
}

Napi::Value RTCPeerConnection::GetReceivers(const Napi::CallbackInfo& info) {
  std::vector<RTCRtpReceiver*> receivers;
  if (_jinglePeerConnection) {
    for (const auto& receiver : _jinglePeerConnection->GetReceivers()) {
      receivers.emplace_back(RTCRtpReceiver::wrap()->GetOrCreate(_factory, receiver));
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), receivers, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::GetSenders(const Napi::CallbackInfo& info) {
  std::vector<RTCRtpSender*> senders;
  if (_jinglePeerConnection) {
    for (const auto& sender : _jinglePeerConnection->GetSenders()) {
      senders.emplace_back(RTCRtpSender::wrap()->GetOrCreate(_factory, sender));
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), senders, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::GetStats(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  CREATE_DEFERRED(env, deferred)

  if (!_jinglePeerConnection) {
    Reject(deferred, ErrorFactory::CreateError(env, "RTCPeerConnection is closed"));
    return deferred.Promise();
  }

  auto callback = new rtc::RefCountedObject<RTCStatsCollector>(this, deferred);
  _jinglePeerConnection->GetStats(callback);

  return deferred.Promise();  // NOLINT
}

Napi::Value RTCPeerConnection::LegacyGetStats(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  CREATE_DEFERRED(env, deferred)

  if (!_jinglePeerConnection) {
    Reject(deferred, Napi::Error::New(env, "RTCPeerConnection is closed"));
    return deferred.Promise();
  }

  auto statsObserver = new rtc::RefCountedObject<StatsObserver>(this, deferred);
  if (!_jinglePeerConnection->GetStats(statsObserver, nullptr,
          webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
    Reject(deferred, Napi::Error::New(env, "Failed to execute getStats"));
    return deferred.Promise();
  }

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::GetTransceivers(const Napi::CallbackInfo& info) {
  std::vector<RTCRtpTransceiver*> transceivers;
  if (_jinglePeerConnection
      && _jinglePeerConnection->GetConfiguration().sdp_semantics == webrtc::SdpSemantics::kUnifiedPlan) {
    for (const auto& transceiver : _jinglePeerConnection->GetTransceivers()) {
      transceivers.emplace_back(RTCRtpTransceiver::wrap()->GetOrCreate(_factory, transceiver));
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), transceivers, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::UpdateIce(const Napi::CallbackInfo& info) {
  return info.Env().Undefined();
}

Napi::Value RTCPeerConnection::Close(const Napi::CallbackInfo& info) {
  if (_jinglePeerConnection) {
    _cached_configuration = ExtendedRTCConfiguration(
            _jinglePeerConnection->GetConfiguration(),
            _port_range);
    _jinglePeerConnection->Close();
    // NOTE(mroberts): Perhaps another way to do this is to just register all remote MediaStreamTracks against this
    // RTCPeerConnection, not unlike what we do with RTCDataChannels.
    if (_jinglePeerConnection->GetConfiguration().sdp_semantics == webrtc::SdpSemantics::kUnifiedPlan) {
      for (const auto& transceiver : _jinglePeerConnection->GetTransceivers()) {
        auto track = MediaStreamTrack::wrap()->GetOrCreate(_factory, transceiver->receiver()->track());
        track->OnPeerConnectionClosed();
      }
    }
    for (auto channel : _channels) {
      channel->OnPeerConnectionClosed();
    }
  }

  _jinglePeerConnection = nullptr;

  if (_factory) {
    if (_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }

  return info.Env().Undefined();
}

Napi::Value RTCPeerConnection::RestartIce(const Napi::CallbackInfo& info) {
  (void) info;
  if (_jinglePeerConnection) {
    _jinglePeerConnection->RestartIce();
  }
  return info.Env().Undefined();
}

Napi::Value RTCPeerConnection::GetCanTrickleIceCandidates(const Napi::CallbackInfo& info) {
  return info.Env().Null();
}

Napi::Value RTCPeerConnection::GetConnectionState(const Napi::CallbackInfo& info) {
  auto env = info.Env();

  auto connectionState = _jinglePeerConnection
      ? _jinglePeerConnection->peer_connection_state()
      : webrtc::PeerConnectionInterface::PeerConnectionState::kClosed;

  CONVERT_OR_THROW_AND_RETURN_NAPI(env, connectionState, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::GetCurrentLocalDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->current_local_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, _jinglePeerConnection->current_local_description(), description, Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value RTCPeerConnection::GetLocalDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->local_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, _jinglePeerConnection->local_description(), description, Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value RTCPeerConnection::GetPendingLocalDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->pending_local_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, _jinglePeerConnection->pending_local_description(), description, Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value RTCPeerConnection::GetCurrentRemoteDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->current_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, _jinglePeerConnection->current_remote_description(), description, Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value RTCPeerConnection::GetRemoteDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->remote_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, _jinglePeerConnection->remote_description(), description, Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value RTCPeerConnection::GetPendingRemoteDescription(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->pending_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, _jinglePeerConnection->pending_remote_description(), description, Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value RTCPeerConnection::GetSctp(const Napi::CallbackInfo& info) {
  return _jinglePeerConnection && _jinglePeerConnection->GetSctpTransport()
      ? RTCSctpTransport::wrap()->GetOrCreate(_factory, _jinglePeerConnection->GetSctpTransport())->Value()
      : info.Env().Null();
}

Napi::Value RTCPeerConnection::GetSignalingState(const Napi::CallbackInfo& info) {
  auto signalingState = _jinglePeerConnection
      ? _jinglePeerConnection->signaling_state()
      : webrtc::PeerConnectionInterface::SignalingState ::kClosed;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), signalingState, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::GetIceConnectionState(const Napi::CallbackInfo& info) {
  auto iceConnectionState = _jinglePeerConnection
      ? _jinglePeerConnection->standardized_ice_connection_state()
      : webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), iceConnectionState, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::GetIceGatheringState(const Napi::CallbackInfo& info) {
  auto iceGatheringState = _jinglePeerConnection
      ? _jinglePeerConnection->ice_gathering_state()
      : webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), iceGatheringState, result, Napi::Value)
  return result;
}

void RTCPeerConnection::SaveLastSdp(const RTCSessionDescriptionInit& lastSdp) {
  this->_lastSdp = lastSdp;
}

void RTCPeerConnection::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "RTCPeerConnection", {
    InstanceMethod("addTrack", &RTCPeerConnection::AddTrack),
    InstanceMethod("addTransceiver", &RTCPeerConnection::AddTransceiver),
    InstanceMethod("removeTrack", &RTCPeerConnection::RemoveTrack),
    InstanceMethod("createOffer", &RTCPeerConnection::CreateOffer),
    InstanceMethod("createAnswer", &RTCPeerConnection::CreateAnswer),
    InstanceMethod("setLocalDescription", &RTCPeerConnection::SetLocalDescription),
    InstanceMethod("setRemoteDescription", &RTCPeerConnection::SetRemoteDescription),
    InstanceMethod("getConfiguration", &RTCPeerConnection::GetConfiguration),
    InstanceMethod("setConfiguration", &RTCPeerConnection::SetConfiguration),
    InstanceMethod("restartIce", &RTCPeerConnection::RestartIce),
    InstanceMethod("getReceivers", &RTCPeerConnection::GetReceivers),
    InstanceMethod("getSenders", &RTCPeerConnection::GetSenders),
    InstanceMethod("getStats", &RTCPeerConnection::GetStats),
    InstanceMethod("legacyGetStats", &RTCPeerConnection::LegacyGetStats),
    InstanceMethod("getTransceivers", &RTCPeerConnection::GetTransceivers),
    InstanceMethod("updateIce", &RTCPeerConnection::UpdateIce),
    InstanceMethod("addIceCandidate", &RTCPeerConnection::AddIceCandidate),
    InstanceMethod("createDataChannel", &RTCPeerConnection::CreateDataChannel),
    InstanceMethod("close", &RTCPeerConnection::Close),
    InstanceAccessor("canTrickleIceCandidates", &RTCPeerConnection::GetCanTrickleIceCandidates, nullptr),
    InstanceAccessor("connectionState", &RTCPeerConnection::GetConnectionState, nullptr),
    InstanceAccessor("currentLocalDescription", &RTCPeerConnection::GetCurrentLocalDescription, nullptr),
    InstanceAccessor("localDescription", &RTCPeerConnection::GetLocalDescription, nullptr),
    InstanceAccessor("pendingLocalDescription", &RTCPeerConnection::GetPendingLocalDescription, nullptr),
    InstanceAccessor("currentRemoteDescription", &RTCPeerConnection::GetCurrentRemoteDescription, nullptr),
    InstanceAccessor("remoteDescription", &RTCPeerConnection::GetRemoteDescription, nullptr),
    InstanceAccessor("pendingRemoteDescription", &RTCPeerConnection::GetPendingRemoteDescription, nullptr),
    InstanceAccessor("sctp", &RTCPeerConnection::GetSctp, nullptr),
    InstanceAccessor("signalingState", &RTCPeerConnection::GetSignalingState, nullptr),
    InstanceAccessor("iceConnectionState", &RTCPeerConnection::GetIceConnectionState, nullptr),
    InstanceAccessor("iceGatheringState", &RTCPeerConnection::GetIceGatheringState, nullptr)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCPeerConnection", func);
}

}  // namespace node_webrtc
