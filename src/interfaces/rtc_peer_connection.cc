/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_peer_connection.hh"

#include <iostream>
#include <src/api/jsep.h>
#include <webrtc/api/media_types.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/rtc_error.h>
#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/p2p/client/basic_port_allocator.h>

#include "src/converters.hh"
#include "src/converters/absl.hh" // IWYU pragma: keep. Needed for conversions
#include "src/converters/arguments.hh"
#include "src/converters/interfaces.hh" // IWYU pragma: keep. Needed for conversions.
#include "src/converters/napi.hh"
#include "src/dictionaries/macros/napi.hh"
#include "src/dictionaries/node_webrtc/rtc_answer_options.hh"
#include "src/dictionaries/node_webrtc/rtc_offer_options.hh"
#include "src/dictionaries/node_webrtc/rtc_session_description_init.hh"
#include "src/dictionaries/node_webrtc/some_error.hh"
#include "src/dictionaries/webrtc/data_channel_init.hh"
#include "src/dictionaries/webrtc/ice_candidate_interface.hh"
#include "src/dictionaries/webrtc/rtc_configuration.hh" // IWYU pragma: keep. Needed for conversions.
#include "src/dictionaries/webrtc/rtc_error.hh"
#include "src/dictionaries/webrtc/rtp_transceiver_init.hh"
#include "src/enums/webrtc/ice_connection_state.hh" // IWYU pragma: keep. Needed for conversions
#include "src/enums/webrtc/ice_gathering_state.hh" // IWYU pragma: keep. Needed for conversions
#include "src/enums/webrtc/media_type.hh" // IWYU pragma: keep. Needed for conversions.
#include "src/enums/webrtc/peer_connection_state.hh" // IWYU pragma: keep. Needed for conversions.
#include "src/enums/webrtc/signaling_state.hh" // IWYU pragma: keep. Needed for conversions.
#include "src/functional/either.hh"
#include "src/functional/maybe.hh"
#include "src/interfaces/media_stream.hh"
#include "src/interfaces/media_stream_track.hh"
#include "src/interfaces/rtc_data_channel.hh"
#include "src/interfaces/rtc_peer_connection/create_session_description_observer.hh"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.hh"
#include "src/interfaces/rtc_peer_connection/rtc_stats_collector.hh"
#include "src/interfaces/rtc_peer_connection/set_session_description_observer.hh"
#include "src/interfaces/rtc_rtp_receiver.hh"
#include "src/interfaces/rtc_rtp_sender.hh"
#include "src/interfaces/rtc_rtp_transceiver.hh"
#include "src/interfaces/rtc_sctp_transport.hh"
#include "src/node/error_factory.hh"
#include "src/node/events.hh"
#include "src/node/promise.hh"
#include "src/node/ref_ptr.hh"
#include "src/node/utility.hh"

namespace node_webrtc {

Napi::FunctionReference &RTCPeerConnection::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

//
// PeerConnection
//

RTCPeerConnection::RTCPeerConnection(const Napi::CallbackInfo &info)
    : AsyncObjectWrapWithLoop<RTCPeerConnection>("RTCPeerConnection", *this,
                                                 info) {
  auto env = info.Env();

  if (!info.IsConstructCall()) {
    Napi::TypeError::New(
        env, "Use the new operator to construct the RTCPeerConnection.")
        .ThrowAsJavaScriptException();
    return;
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN_VOID_NAPI(info, maybeConfiguration,
                                             Maybe<ExtendedRTCConfiguration>)

  auto configuration = maybeConfiguration.FromMaybe(ExtendedRTCConfiguration());

  // TODO(mroberts): Read `factory` (non-standard) from RTCConfiguration?
  _factory = PeerConnectionFactory::GetOrCreateDefault();
  _shouldReleaseFactory = true;

  auto portAllocator =
      std::unique_ptr<cricket::PortAllocator>(new cricket::BasicPortAllocator(
          _factory->getNetworkManager(), _factory->getSocketFactory()));
  _port_range = configuration.portRange;
  portAllocator->SetPortRange(_port_range.min.FromMaybe(0),
                              _port_range.max.FromMaybe(65535));

  auto deps = webrtc::PeerConnectionDependencies(this);
  deps.allocator = std::move(portAllocator);
  auto maybePeerConnection = _factory->factory()->CreatePeerConnectionOrError(
      configuration.configuration, std::move(deps));

  if (!maybePeerConnection.ok()) {
    Napi::Error::New(env, maybePeerConnection.MoveError().message())
        .ThrowAsJavaScriptException();
    return;
  }

  _jinglePeerConnection = maybePeerConnection.MoveValue();
}

RTCPeerConnection::~RTCPeerConnection() {
  // Clear the wrap caches before releasing the WebRTC peer connection.
  // This releases all scoped_refptr references to WebRTC objects while
  // the signaling thread may still be active, preventing crashes during
  // cleanup.
  _data_channel_wrap.clear();
  _stream_wrap.clear();
  _receiver_wrap.clear();
  _transceiver_wrap.clear();
  _sender_wrap.clear();
  _transport_wrap.clear();
  _channels.clear();

  _jinglePeerConnection = nullptr;
  if (_factory) {
    if (_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }
}

void RTCPeerConnection::OnSignalingChange(
    webrtc::PeerConnectionInterface::SignalingState state) {
  Dispatch(CreateCallback<RTCPeerConnection>([this, state]() {
    MakeCallback("onsignalingstatechange", {});
    if (state == webrtc::PeerConnectionInterface::kClosed) {
      Stop();
    }
  }));
}

void RTCPeerConnection::OnIceConnectionChange(
    webrtc::PeerConnectionInterface::IceConnectionState) {
  Dispatch(CreateCallback<RTCPeerConnection>([this]() {
    MakeCallback("oniceconnectionstatechange", {});
    MakeCallback("onconnectionstatechange", {});
  }));
}

void RTCPeerConnection::OnIceGatheringChange(
    webrtc::PeerConnectionInterface::IceGatheringState) {
  Dispatch(CreateCallback<RTCPeerConnection>(
      [this]() { MakeCallback("onicegatheringstatechange", {}); }));
}

void RTCPeerConnection::OnIceCandidate(
    const webrtc::IceCandidateInterface *ice_candidate) {
  std::string error;

  std::string sdp;
  if (!ice_candidate->ToString(&sdp)) {
    error = "Failed to print the candidate string. This is pretty weird. File "
            "a bug on https://github.com/node-webrtc/node-webrtc";
    return;
  }

  webrtc::SdpParseError parseError;
  auto candidate =
      std::shared_ptr<webrtc::IceCandidateInterface>(webrtc::CreateIceCandidate(
          ice_candidate->sdp_mid(), ice_candidate->sdp_mline_index(), sdp,
          &parseError));
  if (!parseError.description.empty()) {
    error = parseError.description;
  } else if (!candidate) {
    error = "Failed to copy RTCIceCandidate";
  }

  Dispatch(CreateCallback<RTCPeerConnection>([this, candidate, error]() {
    if (error.empty()) {
      auto env = Env();
      auto maybeCandidate =
          From<Napi::Value>(std::make_pair(env, candidate.get()));
      if (maybeCandidate.IsValid()) {
        MakeCallback("onicecandidate", {maybeCandidate.UnsafeFromValid()});
      }
    }
  }));
}

static Validation<Napi::Value> CreateRTCPeerConnectionIceErrorEvent(
    const Napi::Value address, const Napi::Value port, const Napi::Value url,
    const Napi::Value errorCode, const Napi::Value errorText) {
  auto env = address.Env();
  Napi::EscapableHandleScope scope(env);
  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "address", address)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "port", port)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "url", url)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "errorCode", errorCode)
  NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "errorText", errorText)
  return Pure(scope.Escape(object));
}

void RTCPeerConnection::OnIceCandidateError(const std::string &address,
                                            int port, const std::string &url,
                                            int error_code,
                                            const std::string &error_text) {
  Dispatch(CreateCallback<RTCPeerConnection>(
      [this, address, port, url, error_code, error_text]() {
        auto env = Env();
        auto maybeEvent = Validation<Napi::Value>::Join(
            curry(CreateRTCPeerConnectionIceErrorEvent) %
            From<Napi::Value>(std::make_pair(env, address)) *
            From<Napi::Value>(std::make_pair(env, port)) *
            From<Napi::Value>(std::make_pair(env, url)) *
            From<Napi::Value>(std::make_pair(env, error_code)) *
            From<Napi::Value>(std::make_pair(env, error_text)));
        if (maybeEvent.IsValid()) {
          MakeCallback("onicecandidateerror", {maybeEvent.UnsafeFromValid()});
        }
      }));
}

void RTCPeerConnection::OnDataChannel(
    rtc::scoped_refptr<webrtc::DataChannelInterface> channel) {
  auto observer = new DataChannelObserver(_factory, channel);
  Dispatch(CreateCallback<RTCPeerConnection>([this, observer]() {
    auto channel =
        _data_channel_wrap.GetOrCreate(observer, observer->channel());
    MakeCallback("ondatachannel", {channel->Value()});
  }));
}

void RTCPeerConnection::OnAddStream(
    rtc::scoped_refptr<webrtc::MediaStreamInterface>) {}

void RTCPeerConnection::OnAddTrack(
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver,
    const std::vector<rtc::scoped_refptr<webrtc::MediaStreamInterface>>
        &streams) {
// TODO(jack): once libwebrtc fully deprecates kPlanB, we can remove this
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
  if (_jinglePeerConnection->GetConfiguration().sdp_semantics !=
      webrtc::SdpSemantics::kPlanB) {
    return;
  }
#pragma clang diagnostic pop
  Dispatch(CreateCallback<RTCPeerConnection>([this, receiver, streams]() {
    if (_factory == nullptr) {
      // We have closed, but have not processed close event to stop the event
      // loop yet. In that case, we should not be broadcasting this event
      return;
    }
    auto mediaStreams = std::vector<MediaStream *>();
    for (auto const &stream : streams) {
      auto mediaStream = _stream_wrap.GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN_VOID_NAPI(Env(), mediaStreams, streamArray,
                                          Napi::Value)
    MakeCallback("ontrack",
                 {_receiver_wrap.GetOrCreate(_factory, receiver)->Value(),
                  streamArray, Env().Null()});
  }));
}

void RTCPeerConnection::OnTrack(
    rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {
  auto receiver = transceiver->receiver();
  auto streams = receiver->streams();
  Dispatch(CreateCallback<RTCPeerConnection>([this, transceiver, receiver,
                                              streams]() {
    if (_factory == nullptr) {
      // We have closed, but have not processed close event to stop the
      // event loop yet. In that case, we should not be broadcasting this
      // event
      return;
    }
    auto mediaStreams = std::vector<MediaStream *>();
    for (auto const &stream : streams) {
      auto mediaStream = _stream_wrap.GetOrCreate(_factory, stream);
      mediaStreams.push_back(mediaStream);
    }
    CONVERT_OR_THROW_AND_RETURN_VOID_NAPI(Env(), mediaStreams, streamArray,
                                          Napi::Value)
    MakeCallback(
        "ontrack",
        {_receiver_wrap.GetOrCreate(_factory, receiver)->Value(), streamArray,
         _transceiver_wrap.GetOrCreate(_factory, transceiver)->Value()});
  }));
}

void RTCPeerConnection::OnRemoveStream(
    rtc::scoped_refptr<webrtc::MediaStreamInterface>) {}

void RTCPeerConnection::OnRenegotiationNeeded() {
  Dispatch(CreateCallback<RTCPeerConnection>(
      [this]() { MakeCallback("onnegotiationneeded", {}); }));
}

Napi::Value RTCPeerConnection::AddTrack(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(
                         env, "Cannot addTrack; RTCPeerConnection is closed"))
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(
      info, pair,
      std::tuple<MediaStreamTrack * COMMA Maybe<std::vector<MediaStream *>>>)
  auto mediaStreamTrack = std::get<0>(pair);
  Maybe<std::vector<MediaStream *>> mediaStreams = std::get<1>(pair);
  std::vector<std::string> streamIds;
  if (mediaStreams.IsJust()) {
    streamIds.reserve(mediaStreams.UnsafeFromJust().size());
    for (auto const &stream : mediaStreams.UnsafeFromJust()) {
      streamIds.emplace_back(stream->stream()->id());
    }
  }
  auto result =
      _jinglePeerConnection->AddTrack(mediaStreamTrack->track(), streamIds);
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, &result.error(), error, Napi::Value)
    Napi::Error(env, error).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  auto rtpSender = result.value();
  return _sender_wrap.GetOrCreate(_factory, rtpSender)->Value();
}

Napi::Value RTCPeerConnection::AddTransceiver(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    Napi::Error::New(env, "Cannot addTransceiver; RTCPeerConnection is closed")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  if (_jinglePeerConnection->GetConfiguration().sdp_semantics !=
      webrtc::SdpSemantics::kUnifiedPlan) {
    Napi::Error::New(
        env,
        "AddTransceiver is only available with Unified Plan SdpSemanticsAbort")
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(
      info, args,
      std::tuple<Either<cricket::MediaType COMMA MediaStreamTrack *> COMMA
                     Maybe<webrtc::RtpTransceiverInit>>)
  Either<cricket::MediaType, MediaStreamTrack *> kindOrTrack =
      std::get<0>(args);
  Maybe<webrtc::RtpTransceiverInit> maybeInit = std::get<1>(args);
  auto result =
      kindOrTrack.IsLeft()
          ? maybeInit.IsNothing()
                ? _jinglePeerConnection->AddTransceiver(
                      kindOrTrack.UnsafeFromLeft())
                : _jinglePeerConnection->AddTransceiver(
                      kindOrTrack.UnsafeFromLeft(), maybeInit.UnsafeFromJust())
      : maybeInit.IsNothing() ? _jinglePeerConnection->AddTransceiver(
                                    kindOrTrack.UnsafeFromRight()->track())
                              : _jinglePeerConnection->AddTransceiver(
                                    kindOrTrack.UnsafeFromRight()->track(),
                                    maybeInit.UnsafeFromJust());
  if (!result.ok()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env, &result.error(), error, Napi::Value)
    Napi::Error(env, error).ThrowAsJavaScriptException();
    return env.Undefined();
  }
  auto rtpTransceiver = result.value();
  return _transceiver_wrap.GetOrCreate(_factory, rtpTransceiver)->Value();
}

Napi::Value RTCPeerConnection::RemoveTrack(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    Napi::Error(env,
                ErrorFactory::CreateInvalidStateError(
                    env, "Cannot removeTrack; RTCPeerConnection is closed"))
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, sender, RTCRtpSender *)
  auto senders = _jinglePeerConnection->GetSenders();
  if (std::find(senders.begin(), senders.end(), sender->sender()) ==
      senders.end()) {
    Napi::Error(
        env, ErrorFactory::CreateInvalidAccessError(env, "Cannot removeTrack"))
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  auto error = _jinglePeerConnection->RemoveTrackOrError(sender->sender());
  if (!error.ok()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), &error, result, Napi::Value)
    Napi::Error(info.Env(), result).ThrowAsJavaScriptException();
  }
  return env.Undefined();
}

Napi::Value RTCPeerConnection::CreateOffer(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  auto maybeOptions =
      From<Maybe<RTCOfferOptions>>(Arguments(info)).Map([](auto maybeOptions) {
        return maybeOptions.FromMaybe(RTCOfferOptions());
      });
  if (maybeOptions.IsInvalid()) {
    Reject(deferred, SomeError(maybeOptions.ToErrors()[0]));
    return deferred.Promise();
  }

  if (!_jinglePeerConnection ||
      _jinglePeerConnection->signaling_state() ==
          webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(deferred,
           ErrorFactory::CreateInvalidStateError(
               env, "Failed to execute 'createOffer' on 'RTCPeerConnection': "
                    "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto observer =
      rtc::make_ref_counted<CreateSessionDescriptionObserver>(this, deferred);
  _jinglePeerConnection->CreateOffer(observer.get(),
                                     maybeOptions.UnsafeFromValid().options);

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::CreateAnswer(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, maybeOptions,
                                         Maybe<RTCAnswerOptions>)

  if (!_jinglePeerConnection ||
      _jinglePeerConnection->signaling_state() ==
          webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(deferred,
           ErrorFactory::CreateInvalidStateError(
               env, "Failed to execute 'createAnswer' on 'RTCPeerConnection': "
                    "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto options = maybeOptions.Map([](auto options) { return options.options; })
                     .OrDefault();
  auto observer = new rtc::RefCountedObject<CreateSessionDescriptionObserver>(
      this, deferred);
  _jinglePeerConnection->CreateAnswer(observer, options);

  return deferred.Promise();
}

Napi::Value
RTCPeerConnection::SetLocalDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, descriptionInit,
                                         RTCSessionDescriptionInit)
  if (descriptionInit.sdp.empty()) {
    descriptionInit.sdp = _lastSdp.sdp;
  }

  auto maybeRawDescription =
      From<webrtc::SessionDescriptionInterface *>(descriptionInit);
  if (maybeRawDescription.IsInvalid()) {
    Reject(deferred, maybeRawDescription.ToErrors()[0]);
    return deferred.Promise();
  }
  auto rawDescription = maybeRawDescription.UnsafeFromValid();
  std::unique_ptr<webrtc::SessionDescriptionInterface> description(
      rawDescription);

  if (!_jinglePeerConnection ||
      _jinglePeerConnection->signaling_state() ==
          webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(
        deferred,
        ErrorFactory::CreateInvalidStateError(
            env,
            "Failed to execute 'setLocalDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto observer =
      new rtc::RefCountedObject<SetSessionDescriptionObserver>(this, deferred);
  _jinglePeerConnection->SetLocalDescription(observer, description.release());

  return deferred.Promise();
}

Napi::Value
RTCPeerConnection::SetRemoteDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, rawDescription,
                                         webrtc::SessionDescriptionInterface *)
  std::unique_ptr<webrtc::SessionDescriptionInterface> description(
      rawDescription);

  if (!_jinglePeerConnection ||
      _jinglePeerConnection->signaling_state() ==
          webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Reject(
        deferred,
        ErrorFactory::CreateInvalidStateError(
            env,
            "Failed to execute 'setRemoteDescription' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signalingState is 'closed'."));
    return deferred.Promise();
  }

  auto observer =
      rtc::make_ref_counted<SetSessionDescriptionObserver>(this, deferred);
  _jinglePeerConnection->SetRemoteDescription(observer.get(),
                                              description.release());

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::AddIceCandidate(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  CREATE_DEFERRED(env, deferred)

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(
      deferred, info, maybeCandidate,
      Maybe<std::shared_ptr<webrtc::IceCandidateInterface>>)

  Dispatch(CreatePromise<RTCPeerConnection>(deferred, [this, maybeCandidate](
                                                          auto deferred) {
    if (!_jinglePeerConnection) {
      Reject(deferred,
             SomeError(
                 "Failed to set ICE candidate; RTCPeerConnection is closed."));
      return;
    }
    if (_jinglePeerConnection->signaling_state() ==
        webrtc::PeerConnectionInterface::SignalingState::kClosed) {
      Reject(deferred,
             SomeError("Failed to set ICE candidate; RTCPeerConnection has a "
                       "closed signaling state."));
      return;
    }

    if (maybeCandidate.IsJust()) {
      // TODO(jack): Currently, it doesn't seem the underlying libwebrtc
      // supports end-of-candidates. I hope upgrading fixes this.
      if (!_jinglePeerConnection->AddIceCandidate(
              maybeCandidate.UnsafeFromJust().get())) {
        Reject(deferred, SomeError("Failed to set ICE candidate."));
        return;
      }
    }

    Resolve(deferred, this->Env().Undefined());
  }));

  return deferred.Promise();
}

Napi::Value
RTCPeerConnection::CreateDataChannel(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    Napi::Error(
        env,
        ErrorFactory::CreateInvalidStateError(
            env,
            "Failed to execute 'createDataChannel' on 'RTCPeerConnection': "
            "The RTCPeerConnection is closed."))
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  if (_jinglePeerConnection->signaling_state() ==
      webrtc::PeerConnectionInterface::SignalingState::kClosed) {
    Napi::Error(
        env,
        ErrorFactory::CreateInvalidStateError(
            env,
            "Failed to execute 'createDataChannel' on 'RTCPeerConnection': "
            "The RTCPeerConnection's signaling state is 'closed'."))
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(
      info, args, std::tuple<std::string COMMA Maybe<webrtc::DataChannelInit>>)

  auto label = std::get<0>(args);
  auto dataChannelInit = std::get<1>(args).FromMaybe(webrtc::DataChannelInit());

  auto maybeDataChannel =
      _jinglePeerConnection->CreateDataChannelOrError(label, &dataChannelInit);
  if (!maybeDataChannel.ok()) {
    // TODO: add error message
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(
                         env, "'createDataChannel' failed"))
        .ThrowAsJavaScriptException();
    return env.Undefined();
  }
  auto dataChannel = maybeDataChannel.MoveValue();

  // This memory is freed in rtc_data_channel.cc, in the constructor.
  auto observer = new DataChannelObserver(_factory, dataChannel);
  auto channel =
      _data_channel_wrap.GetOrCreate(observer, observer->channel()); // NOLINT
  _channels.emplace_back(channel);

  return channel->Value();
}

Napi::Value
RTCPeerConnection::GetConfiguration(const Napi::CallbackInfo &info) {
  auto configuration =
      _jinglePeerConnection
          ? ExtendedRTCConfiguration(_jinglePeerConnection->GetConfiguration(),
                                     _port_range)
          : _cached_configuration;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), configuration, result,
                                   Napi::Value)
  return result;
}

Napi::Value
RTCPeerConnection::SetConfiguration(const Napi::CallbackInfo &info) {
  auto env = info.Env();

  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(
      info, configuration, webrtc::PeerConnectionInterface::RTCConfiguration)

  if (!_jinglePeerConnection) {
    Napi::Error(env, ErrorFactory::CreateInvalidStateError(
                         env, "RTCPeerConnection is closed"))
        .ThrowAsJavaScriptException();
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

Napi::Value RTCPeerConnection::GetReceivers(const Napi::CallbackInfo &info) {
  std::vector<RTCRtpReceiver *> receivers;
  if (_jinglePeerConnection) {
    for (const auto &receiver : _jinglePeerConnection->GetReceivers()) {
      auto new_receiver = _receiver_wrap.GetOrCreate(_factory, receiver);
      receivers.emplace_back(new_receiver);
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), receivers, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::GetSenders(const Napi::CallbackInfo &info) {
  std::vector<RTCRtpSender *> senders;
  if (_jinglePeerConnection) {
    for (const auto &sender : _jinglePeerConnection->GetSenders()) {
      senders.emplace_back(_sender_wrap.GetOrCreate(_factory, sender));
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), senders, result, Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::GetStats(const Napi::CallbackInfo &info) {
  auto env = info.Env();

  CREATE_DEFERRED(env, deferred)

  if (!_jinglePeerConnection) {
    Reject(deferred,
           ErrorFactory::CreateError(env, "RTCPeerConnection is closed"));
    return deferred.Promise();
  }

  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, maybeSelector,
                                         Maybe<MediaStreamTrack *>)

  auto callback = rtc::make_ref_counted<RTCStatsCollector>(this, deferred);
  if (maybeSelector.IsJust()) {
    auto selector = maybeSelector.UnsafeFromJust();
    auto track = selector->track();
    // A little silly that the Javascript API is based off Tracks, but the
    // internal API needs an explicit sender or receiver.
    // Copying [what blink
    // does](https://chromium.googlesource.com/chromium/src/+/refs/tags/114.0.5735.227/third_party/blink/renderer/modules/peerconnection/rtc_peer_connection.cc#1791)
    // and iterating through all senders and receivers until we find a match.
    size_t track_uses = 0U;

    rtc::scoped_refptr<webrtc::RtpSenderInterface> track_sender = nullptr;
    for (const auto &sender : _jinglePeerConnection->GetSenders()) {
      if (sender->track() == track) {
        ++track_uses;
        track_sender = sender;
      }
    }

    rtc::scoped_refptr<webrtc::RtpReceiverInterface> track_receiver = nullptr;
    for (const auto &receiver : _jinglePeerConnection->GetReceivers()) {
      if (receiver->track() == track) {
        ++track_uses;
        track_receiver = receiver;
      }
    }

    if (track_uses == 0U) {
      Reject(deferred,
             ErrorFactory::CreateInvalidAccessError(
                 env, "There is no sender or receiver for the track."));
      return deferred.Promise();
    }

    if (track_uses > 1U) {
      Reject(deferred,
             ErrorFactory::CreateInvalidAccessError(
                 env,
                 "There are more than one sender or receiver for the track."));
      return deferred.Promise();
    }

    if (track_sender != nullptr) {
      assert(track_receiver == nullptr);
      _jinglePeerConnection->GetStats(track_sender, callback);
    } else {
      assert(track_receiver != nullptr);
      _jinglePeerConnection->GetStats(track_receiver, callback);
    }
  } else {
    // null selector
    _jinglePeerConnection->GetStats(callback.get());
  }

  return deferred.Promise();
}

Napi::Value RTCPeerConnection::GetTransceivers(const Napi::CallbackInfo &info) {
  std::vector<RTCRtpTransceiver *> transceivers;
  if (_jinglePeerConnection &&
      _jinglePeerConnection->GetConfiguration().sdp_semantics ==
          webrtc::SdpSemantics::kUnifiedPlan) {
    for (const auto &transceiver : _jinglePeerConnection->GetTransceivers()) {
      transceivers.emplace_back(
          _transceiver_wrap.GetOrCreate(_factory, transceiver));
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), transceivers, result,
                                   Napi::Value)
  return result;
}

Napi::Value RTCPeerConnection::UpdateIce(const Napi::CallbackInfo &info) {
  return info.Env().Undefined();
}

Napi::Value RTCPeerConnection::Close(const Napi::CallbackInfo &info) {
  if (_jinglePeerConnection) {
    _cached_configuration = ExtendedRTCConfiguration(
        _jinglePeerConnection->GetConfiguration(), _port_range);
    _jinglePeerConnection->Close();
    // NOTE(mroberts): Perhaps another way to do this is to just register all
    // remote MediaStreamTracks against this RTCPeerConnection, not unlike what
    // we do with RTCDataChannels.
    if (_jinglePeerConnection->GetConfiguration().sdp_semantics ==
        webrtc::SdpSemantics::kUnifiedPlan) {
      for (const auto &transceiver : _jinglePeerConnection->GetTransceivers()) {
        auto track = MediaStreamTrack::wrap()->GetOrCreate(
            _factory, transceiver->receiver()->track());
        track->OnPeerConnectionClosed();
      }
    }
    for (auto channel : _channels) {
      channel->OnPeerConnectionClosed();
    }
  }

  // Clear the wrap caches before releasing the WebRTC peer connection.
  // This releases all scoped_refptr references to WebRTC objects while
  // the signaling thread is still active, preventing crashes during cleanup.
  _data_channel_wrap.clear();
  _stream_wrap.clear();
  _receiver_wrap.clear();
  _transceiver_wrap.clear();
  _sender_wrap.clear();
  _transport_wrap.clear();
  _channels.clear();

  _jinglePeerConnection = nullptr;

  if (_factory) {
    if (_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }

  return info.Env().Undefined();
}

Napi::Value RTCPeerConnection::RestartIce(const Napi::CallbackInfo &info) {
  (void)info;
  if (_jinglePeerConnection) {
    _jinglePeerConnection->RestartIce();
  }
  return info.Env().Undefined();
}

Napi::Value
RTCPeerConnection::GetCanTrickleIceCandidates(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  if (!_jinglePeerConnection) {
    return env.Null();
  }
  auto canTrickleIceCandidates =
      _jinglePeerConnection->can_trickle_ice_candidates();
  CONVERT_OR_THROW_AND_RETURN_NAPI(env, canTrickleIceCandidates, result,
                                   Napi::Value)
  return result;
}

Napi::Value
RTCPeerConnection::GetConnectionState(const Napi::CallbackInfo &info) {
  auto env = info.Env();

  auto connectionState =
      _jinglePeerConnection
          ? _jinglePeerConnection->peer_connection_state()
          : webrtc::PeerConnectionInterface::PeerConnectionState::kClosed;

  CONVERT_OR_THROW_AND_RETURN_NAPI(env, connectionState, result, Napi::Value)
  return result;
}

Napi::Value
RTCPeerConnection::GetCurrentLocalDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection &&
      _jinglePeerConnection->current_local_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(
        env, _jinglePeerConnection->current_local_description(), description,
        Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value
RTCPeerConnection::GetLocalDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->local_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(env,
                                     _jinglePeerConnection->local_description(),
                                     description, Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value
RTCPeerConnection::GetPendingLocalDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection &&
      _jinglePeerConnection->pending_local_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(
        env, _jinglePeerConnection->pending_local_description(), description,
        Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value
RTCPeerConnection::GetCurrentRemoteDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection &&
      _jinglePeerConnection->current_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(
        env, _jinglePeerConnection->current_remote_description(), description,
        Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value
RTCPeerConnection::GetRemoteDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection && _jinglePeerConnection->remote_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(
        env, _jinglePeerConnection->remote_description(), description,
        Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value
RTCPeerConnection::GetPendingRemoteDescription(const Napi::CallbackInfo &info) {
  auto env = info.Env();
  Napi::Value result = env.Null();
  if (_jinglePeerConnection &&
      _jinglePeerConnection->pending_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN_NAPI(
        env, _jinglePeerConnection->pending_remote_description(), description,
        Napi::Value)
    result = description;
  }
  return result;
}

Napi::Value RTCPeerConnection::GetSctp(const Napi::CallbackInfo &info) {
  return _jinglePeerConnection && _jinglePeerConnection->GetSctpTransport()
             ? _transport_wrap
                   .GetOrCreate(_factory,
                                _jinglePeerConnection->GetSctpTransport())
                   ->Value()
             : info.Env().Null();
}

Napi::Value
RTCPeerConnection::GetSignalingState(const Napi::CallbackInfo &info) {
  auto signalingState =
      _jinglePeerConnection
          ? _jinglePeerConnection->signaling_state()
          : webrtc::PeerConnectionInterface::SignalingState ::kClosed;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), signalingState, result,
                                   Napi::Value)
  return result;
}

Napi::Value
RTCPeerConnection::GetIceConnectionState(const Napi::CallbackInfo &info) {
  auto iceConnectionState =
      _jinglePeerConnection
          ? _jinglePeerConnection->standardized_ice_connection_state()
          : webrtc::PeerConnectionInterface::IceConnectionState::
                kIceConnectionClosed;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), iceConnectionState, result,
                                   Napi::Value)
  return result;
}

Napi::Value
RTCPeerConnection::GetIceGatheringState(const Napi::CallbackInfo &info) {
  auto iceGatheringState = _jinglePeerConnection
                               ? _jinglePeerConnection->ice_gathering_state()
                               : webrtc::PeerConnectionInterface::
                                     IceGatheringState::kIceGatheringComplete;
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), iceGatheringState, result,
                                   Napi::Value)
  return result;
}

void RTCPeerConnection::SaveLastSdp(const RTCSessionDescriptionInit &lastSdp) {
  this->_lastSdp = lastSdp;
}

void RTCPeerConnection::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(
      env, "RTCPeerConnection",
      {InstanceMethod("addTrack", &RTCPeerConnection::AddTrack),
       InstanceMethod("addTransceiver", &RTCPeerConnection::AddTransceiver),
       InstanceMethod("removeTrack", &RTCPeerConnection::RemoveTrack),
       InstanceMethod("createOffer", &RTCPeerConnection::CreateOffer),
       InstanceMethod("createAnswer", &RTCPeerConnection::CreateAnswer),
       InstanceMethod("setLocalDescription",
                      &RTCPeerConnection::SetLocalDescription),
       InstanceMethod("setRemoteDescription",
                      &RTCPeerConnection::SetRemoteDescription),
       InstanceMethod("getConfiguration", &RTCPeerConnection::GetConfiguration),
       InstanceMethod("setConfiguration", &RTCPeerConnection::SetConfiguration),
       InstanceMethod("restartIce", &RTCPeerConnection::RestartIce),
       InstanceMethod("getReceivers", &RTCPeerConnection::GetReceivers),
       InstanceMethod("getSenders", &RTCPeerConnection::GetSenders),
       InstanceMethod("getStats", &RTCPeerConnection::GetStats),
       InstanceMethod("getTransceivers", &RTCPeerConnection::GetTransceivers),
       InstanceMethod("updateIce", &RTCPeerConnection::UpdateIce),
       InstanceMethod("addIceCandidate", &RTCPeerConnection::AddIceCandidate),
       InstanceMethod("createDataChannel",
                      &RTCPeerConnection::CreateDataChannel),
       InstanceMethod("close", &RTCPeerConnection::Close),
       InstanceAccessor("canTrickleIceCandidates",
                        &RTCPeerConnection::GetCanTrickleIceCandidates,
                        nullptr),
       InstanceAccessor("connectionState",
                        &RTCPeerConnection::GetConnectionState, nullptr),
       InstanceAccessor("currentLocalDescription",
                        &RTCPeerConnection::GetCurrentLocalDescription,
                        nullptr),
       InstanceAccessor("localDescription",
                        &RTCPeerConnection::GetLocalDescription, nullptr),
       InstanceAccessor("pendingLocalDescription",
                        &RTCPeerConnection::GetPendingLocalDescription,
                        nullptr),
       InstanceAccessor("currentRemoteDescription",
                        &RTCPeerConnection::GetCurrentRemoteDescription,
                        nullptr),
       InstanceAccessor("remoteDescription",
                        &RTCPeerConnection::GetRemoteDescription, nullptr),
       InstanceAccessor("pendingRemoteDescription",
                        &RTCPeerConnection::GetPendingRemoteDescription,
                        nullptr),
       InstanceAccessor("sctp", &RTCPeerConnection::GetSctp, nullptr),
       InstanceAccessor("signalingState", &RTCPeerConnection::GetSignalingState,
                        nullptr),
       InstanceAccessor("iceConnectionState",
                        &RTCPeerConnection::GetIceConnectionState, nullptr),
       InstanceAccessor("iceGatheringState",
                        &RTCPeerConnection::GetIceGatheringState, nullptr)});

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCPeerConnection", func);
}

} // namespace node_webrtc
