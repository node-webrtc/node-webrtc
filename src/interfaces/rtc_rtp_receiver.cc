/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_receiver.hh"

#include <iostream>
#include <webrtc/api/rtp_receiver_interface.h>

#include "src/converters.hh"
#include "src/converters/arguments.hh"
#include "src/converters/interfaces.hh"
#include "src/converters/napi.hh"
#include "src/dictionaries/webrtc/rtp_capabilities.hh"
#include "src/dictionaries/webrtc/rtp_parameters.hh"
#include "src/dictionaries/webrtc/rtp_source.hh"
#include "src/enums/webrtc/media_type.hh"
#include "src/interfaces/media_stream_track.hh"
#include "src/interfaces/rtc_dtls_transport.hh"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.hh"
#include "src/node/utility.hh"

namespace node_webrtc {

Napi::FunctionReference &RTCRtpReceiver::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCRtpReceiver::RTCRtpReceiver(const Napi::CallbackInfo &info)
    : AsyncObjectWrap<RTCRtpReceiver>("RTCRtpReceiver", info) {
  if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct a RTCRtpReceiver")
        .ThrowAsJavaScriptException();
    return;
  }

  // FIXME(mroberts): There is a safer conversion here.
  _factory = PeerConnectionFactory::Unwrap(info[0].ToObject());
  auto receiver = *info[1]
                       .As<Napi::External<
                           rtc::scoped_refptr<webrtc::RtpReceiverInterface>>>()
                       .Data();

  _receiver = std::move(receiver);
}

RTCRtpReceiver::~RTCRtpReceiver() {
  Napi::HandleScope scope(PeerConnectionFactory::constructor().Env());

  wrap()->Release(this);
}

Napi::Value RTCRtpReceiver::GetTrack(const Napi::CallbackInfo &) {
  return _track_wrap.GetOrCreate(_factory, _receiver->track())->Value();
}

Napi::Value RTCRtpReceiver::GetTransport(const Napi::CallbackInfo &info) {
  auto transport = _receiver->dtls_transport();
  return transport ? _transport_wrap.GetOrCreate(_factory, transport)->Value()
                   : info.Env().Null();
}

Napi::Value RTCRtpReceiver::GetRtcpTransport(const Napi::CallbackInfo &info) {
  return info.Env().Null();
}

Napi::Value RTCRtpReceiver::GetCapabilities(const Napi::CallbackInfo &info) {
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, kindString, std::string)
  if (kindString == "audio" || kindString == "video") {
    auto factory = PeerConnectionFactory::GetOrCreateDefault();
    auto kind = kindString == "audio" ? cricket::MEDIA_TYPE_AUDIO
                                      : cricket::MEDIA_TYPE_VIDEO;
    auto capabilities = factory->factory()->GetRtpReceiverCapabilities(kind);
    CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), capabilities, result,
                                     Napi::Value)
    return result;
  }
  return info.Env().Null();
}

Napi::Value RTCRtpReceiver::GetParameters(const Napi::CallbackInfo &info) {
  auto parameters = _receiver->GetParameters();
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), parameters, result, Napi::Value)
  return result;
}

Napi::Value
RTCRtpReceiver::GetContributingSources(const Napi::CallbackInfo &info) {
  auto contributingSources = std::vector<webrtc::RtpSource>();
  auto sources = _receiver->GetSources();
  for (const auto &source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::CSRC) {
      contributingSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), contributingSources, result,
                                   Napi::Value)
  return result;
}

Napi::Value
RTCRtpReceiver::GetSynchronizationSources(const Napi::CallbackInfo &info) {
  auto synchronizationSources = std::vector<webrtc::RtpSource>();
  auto sources = _receiver->GetSources();
  for (const auto &source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::SSRC) {
      synchronizationSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), synchronizationSources, result,
                                   Napi::Value)
  return result;
}

Napi::Value RTCRtpReceiver::GetStats(const Napi::CallbackInfo &info) {
  CREATE_DEFERRED(info.Env(), deferred)
  Reject(
      deferred,
      Napi::Error::New(
          info.Env(),
          "Not yet implemented; file a feature request against node-webrtc"));
  return deferred.Promise();
}

Wrap<RTCRtpReceiver *, rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
     RefPtr<PeerConnectionFactory>> *
RTCRtpReceiver::wrap() {
  static auto wrap =
      new node_webrtc::Wrap<RTCRtpReceiver *,
                            rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
                            RefPtr<PeerConnectionFactory>>(
          RTCRtpReceiver::Create);
  return wrap;
}

RTCRtpReceiver *RTCRtpReceiver::Create(
    RefPtr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto object = constructor().New(
      {factory->Value(),
       Napi::External<rtc::scoped_refptr<webrtc::RtpReceiverInterface>>::New(
           env, &receiver)});

  auto unwrapped = Unwrap(object);
  return unwrapped;
}

void RTCRtpReceiver::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(
      env, "RTCRtpReceiver",
      {InstanceAccessor("track", &RTCRtpReceiver::GetTrack, nullptr),
       InstanceAccessor("transport", &RTCRtpReceiver::GetTransport, nullptr),
       InstanceAccessor("rtcpTransport", &RTCRtpReceiver::GetRtcpTransport,
                        nullptr),
       InstanceMethod("getParameters", &RTCRtpReceiver::GetParameters),
       InstanceMethod("getContributingSources",
                      &RTCRtpReceiver::GetContributingSources),
       InstanceMethod("getSynchronizationSources",
                      &RTCRtpReceiver::GetSynchronizationSources),
       InstanceMethod("getStats", &RTCRtpReceiver::GetStats),
       StaticMethod("getCapabilities", &RTCRtpReceiver::GetCapabilities)});

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCRtpReceiver", func);
}

FROM_NAPI_IMPL(RTCRtpReceiver *, value) {
  return From<Napi::Object>(value).FlatMap<RTCRtpReceiver *>(
      [](Napi::Object object) {
        auto isRTCRtpReceiver = false;
        napi_instanceof(object.Env(), object,
                        RTCRtpReceiver::constructor().Value(),
                        &isRTCRtpReceiver);

        if (object.Env().IsExceptionPending()) {
          return Validation<RTCRtpReceiver *>::Invalid(
              object.Env().GetAndClearPendingException().Message());
        }
        if (!isRTCRtpReceiver) {
          return Validation<RTCRtpReceiver *>::Invalid(
              "This is not an instance of RTCRtpReceiver");
        }

        return Pure(RTCRtpReceiver::Unwrap(object));
      });
}

TO_NAPI_IMPL(RTCRtpReceiver *, pair) {
  auto v = pair.second->Value().As<Napi::Value>();
  return Pure(v);
}

} // namespace node_webrtc
