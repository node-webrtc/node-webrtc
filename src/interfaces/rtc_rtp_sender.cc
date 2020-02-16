/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_sender.h"

#include <webrtc/api/rtp_parameters.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/interfaces.h"
#include "src/converters/null.h"
#include "src/dictionaries/webrtc/rtc_error.h"
#include "src/dictionaries/webrtc/rtp_capabilities.h"
#include "src/dictionaries/webrtc/rtp_parameters.h"
#include "src/enums/webrtc/media_type.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/media_stream.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"
#include "src/node/error_factory.h"
#include "src/node/utility.h"

namespace node_webrtc {

Napi::FunctionReference& RTCRtpSender::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCRtpSender::RTCRtpSender(const Napi::CallbackInfo& info)
  : AsyncObjectWrap<RTCRtpSender>("RTCRtpSender", info) {
  if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct a RTCRtpSender").ThrowAsJavaScriptException();
    return;
  }

  auto factory = PeerConnectionFactory::Unwrap(info[0].ToObject());
  auto sender = *info[1].As<Napi::External<rtc::scoped_refptr<webrtc::RtpSenderInterface>>>().Data();

  _factory = factory;
  _factory->Ref();

  _sender = std::move(sender);
}

RTCRtpSender::~RTCRtpSender() {
  Napi::HandleScope scope(PeerConnectionFactory::constructor().Env());
  _factory->Unref();
  _factory = nullptr;

  wrap()->Release(this);
}

Napi::Value RTCRtpSender::GetTrack(const Napi::CallbackInfo& info) {
  Napi::Value result = info.Env().Null();
  auto track = _sender->track();
  if (track) {
    result = MediaStreamTrack::wrap()->GetOrCreate(_factory, track)->Value();
  }
  return result;
}

Napi::Value RTCRtpSender::GetTransport(const Napi::CallbackInfo& info) {
  auto transport = _sender->dtls_transport();
  return transport
      ? RTCDtlsTransport::wrap()->GetOrCreate(_factory, transport)->Value()
      : info.Env().Null();
}

Napi::Value RTCRtpSender::GetRtcpTransport(const Napi::CallbackInfo& info) {
  return info.Env().Null();
}

Napi::Value RTCRtpSender::GetCapabilities(const Napi::CallbackInfo& info) {
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, kindString, std::string)
  if (kindString == "audio" || kindString == "video") {
    auto factory = PeerConnectionFactory::GetOrCreateDefault();
    auto kind = kindString == "audio" ? cricket::MEDIA_TYPE_AUDIO : cricket::MEDIA_TYPE_VIDEO;
    auto capabilities = factory->factory()->GetRtpSenderCapabilities(kind);
    factory->Release();
    CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), capabilities, result, Napi::Value)
    return result;
  }
  return info.Env().Null();
}

Napi::Value RTCRtpSender::GetParameters(const Napi::CallbackInfo& info) {
  auto parameters = _sender->GetParameters();
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), parameters, result, Napi::Value)
  return result;
}

Napi::Value RTCRtpSender::SetParameters(const Napi::CallbackInfo& info) {
  CREATE_DEFERRED(info.Env(), deffered)
  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, parameters, webrtc::RtpParameters)
  auto error = _sender->SetParameters(parameters);
  if (error.ok()) {
    deferred.Resolve(info.Env().Undefined());
  } else {
    CONVERT_OR_REJECT_AND_RETURN_NAPI(deferred, &error, reason, Napi::Value)
    deferred.Reject(reason);
  }
  return deferred.Promise();
}

Napi::Value RTCRtpSender::GetStats(const Napi::CallbackInfo& info) {
  CREATE_DEFERRED(info.Env(), deffered)
  Reject(deferred, Napi::Error::New(info.Env(), "Not yet implemented; file a feature request against node-webrtc"));
  return deferred.Promise();
}

Napi::Value RTCRtpSender::ReplaceTrack(const Napi::CallbackInfo& info) {
  CREATE_DEFERRED(info.Env(), deferred)
  CONVERT_ARGS_OR_REJECT_AND_RETURN_NAPI(deferred, info, maybeTrack, Either<Null COMMA MediaStreamTrack*>)
  auto mediaStreamTrack = maybeTrack.FromEither<MediaStreamTrack*>([](auto) {
    return nullptr;
  }, [](auto track) {
    return track;
  });
  auto track = mediaStreamTrack ? mediaStreamTrack->track().get() : nullptr;
  if (track) {
    auto expectedMediaType = track->kind() == webrtc::MediaStreamTrackInterface::kAudioKind
        ? cricket::MediaType::MEDIA_TYPE_AUDIO
        : cricket::MediaType::MEDIA_TYPE_VIDEO;
    if (_sender->media_type() != expectedMediaType) {
      Reject(deferred, Napi::TypeError::New(info.Env(), "Kind does not match").Value().As<Napi::Value>());
      return deferred.Promise();
    }
  }
  _sender->SetTrack(track)
  ? Resolve(deferred, info.Env().Undefined())
  : Reject(deferred, ErrorFactory::CreateInvalidStateError(info.Env(), "Failed to replaceTrack"));
  return deferred.Promise();
}

Napi::Value RTCRtpSender::SetStreams(const Napi::CallbackInfo& info) {
  auto streams = std::vector<std::string>();
  for (size_t i = 0; i < info.Length(); i++) {
    auto value = info[i];
    auto maybeStream = From<MediaStream*>(value);
    if (maybeStream.IsInvalid()) {
      auto error = maybeStream.ToErrors()[0];
      Napi::TypeError::New(info.Env(), error).ThrowAsJavaScriptException();
      return info.Env().Undefined();
    }
    auto stream = maybeStream.UnsafeFromValid();
    streams.emplace_back(stream->stream()->id());
  }
  _sender->SetStreams(streams);
  return info.Env().Undefined();
}

Wrap <
RTCRtpSender*,
rtc::scoped_refptr<webrtc::RtpSenderInterface>,
PeerConnectionFactory*
> * RTCRtpSender::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpSender*,
  rtc::scoped_refptr<webrtc::RtpSenderInterface>,
  PeerConnectionFactory*
  > (RTCRtpSender::Create);
  return wrap;
}

RTCRtpSender* RTCRtpSender::Create(
    PeerConnectionFactory* factory,
    rtc::scoped_refptr<webrtc::RtpSenderInterface> sender) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto object = constructor().New({
    factory->Value(),
    Napi::External<rtc::scoped_refptr<webrtc::RtpSenderInterface>>::New(env, &sender)
  });

  return RTCRtpSender::Unwrap(object);
}

void RTCRtpSender::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "RTCRtpSender", {
    InstanceAccessor("track", &RTCRtpSender::GetTrack, nullptr),
    InstanceAccessor("transport", &RTCRtpSender::GetTransport, nullptr),
    InstanceAccessor("rtcpTransport", &RTCRtpSender::GetRtcpTransport, nullptr),
    InstanceMethod("getParameters", &RTCRtpSender::GetParameters),
    InstanceMethod("setParameters", &RTCRtpSender::SetParameters),
    InstanceMethod("getStats", &RTCRtpSender::GetStats),
    InstanceMethod("replaceTrack", &RTCRtpSender::ReplaceTrack),
    InstanceMethod("setStreams", &RTCRtpSender::SetStreams),
    StaticMethod("getCapabilities", &RTCRtpSender::GetCapabilities)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCRtpSender", func);
}

FROM_NAPI_IMPL(RTCRtpSender*, value) {
  return From<Napi::Object>(value).FlatMap<RTCRtpSender*>([](Napi::Object object) {
    auto isRTCRtpSender = false;
    napi_instanceof(object.Env(), object, RTCRtpSender::constructor().Value(), &isRTCRtpSender);
    if (object.Env().IsExceptionPending()) {
      return Validation<RTCRtpSender*>::Invalid(object.Env().GetAndClearPendingException().Message());
    } else if (!isRTCRtpSender) {
      return Validation<RTCRtpSender*>::Invalid("This is not an instance of RTCRtpSender");
    }
    return Pure(RTCRtpSender::Unwrap(object));
  });
}

TO_NAPI_IMPL(RTCRtpSender*, pair) {
  return Pure(pair.second->Value().As<Napi::Value>());
}

}  // namespace node_webrtc
