/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_sender.h"

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/interfaces.h"
#include "src/converters/null.h"
#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/rtp_parameters.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/node/utility.h"

namespace node_webrtc {

Napi::FunctionReference& RTCRtpSender::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCRtpSender::RTCRtpSender(const Napi::CallbackInfo& info)
  : napi::AsyncObjectWrap<RTCRtpSender>("RTCRtpSender", info) {
  if (info.Length() != 2 || !info[0].IsExternal() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct a RTCRtpSender").ThrowAsJavaScriptException();
    return;
  }

  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[0]))->Value());
  auto sender = *static_cast<rtc::scoped_refptr<webrtc::RtpSenderInterface>*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[1]))->Value());

  _factory = std::move(factory);
  _sender = std::move(sender);
}

RTCRtpSender::~RTCRtpSender() {
  wrap()->Release(this);
}

Napi::Value RTCRtpSender::GetTrack(const Napi::CallbackInfo& info) {
  Napi::Value result = info.Env().Null();
  auto track = _sender->track();
  if (track) {
    result = napi::UnsafeFromV8(info.Env(), MediaStreamTrack::wrap()->GetOrCreate(_factory, track)->ToObject());
  }
  return result;
}

Napi::Value RTCRtpSender::GetTransport(const Napi::CallbackInfo& info) {
  Napi::Value result = info.Env().Null();
  auto transport = _sender->dtls_transport();
  if (transport) {
    result = napi::UnsafeFromV8(info.Env(), RTCDtlsTransport::wrap()->GetOrCreate(_factory, transport)->ToObject());
  }
  return result;
}

Napi::Value RTCRtpSender::GetRtcpTransport(const Napi::CallbackInfo& info) {
  return info.Env().Null();
}

Napi::Value RTCRtpSender::GetCapabilities(const Napi::CallbackInfo& info) {
  Napi::Error::New(info.Env(), "Not yet implemented; file a feature request against node-webrtc").ThrowAsJavaScriptException();
  return info.Env().Undefined();
}

Napi::Value RTCRtpSender::GetParameters(const Napi::CallbackInfo& info) {
  auto parameters = _sender->GetParameters();
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), parameters, result, Napi::Value)
  return result;
}

Napi::Value RTCRtpSender::SetParameters(const Napi::CallbackInfo& info) {
  CREATE_DEFERRED(info.Env(), deffered)
  napi::Reject(deferred, Napi::Error::New(info.Env(), "Not yet implemented; file a feature request against node-webrtc"));
  return deferred.Promise();
}

Napi::Value RTCRtpSender::GetStats(const Napi::CallbackInfo& info) {
  CREATE_DEFERRED(info.Env(), deffered)
  napi::Reject(deferred, Napi::Error::New(info.Env(), "Not yet implemented; file a feature request against node-webrtc"));
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
  _sender->SetTrack(track)
  ? napi::Resolve(deferred, info.Env().Undefined())
  : napi::Reject(deferred, Napi::Error::New(info.Env(), "Failed to replaceTrack"));
  return deferred.Promise();
}

Wrap <
RTCRtpSender*,
rtc::scoped_refptr<webrtc::RtpSenderInterface>,
std::shared_ptr<PeerConnectionFactory>
> * RTCRtpSender::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpSender*,
  rtc::scoped_refptr<webrtc::RtpSenderInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > (RTCRtpSender::Create);
  return wrap;
}

RTCRtpSender* RTCRtpSender::Create(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpSenderInterface> sender) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto factoryExternal = Nan::New<v8::External>(static_cast<void*>(&factory));
  auto senderExternal = Nan::New<v8::External>(static_cast<void*>(&sender));

  auto object = constructor().New({
    napi::UnsafeFromV8(env, factoryExternal),
    napi::UnsafeFromV8(env, senderExternal)
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

FROM_JS_IMPL(RTCRtpSender*, value) {
  return From<RTCRtpSender*>(napi::UnsafeFromV8(RTCRtpSender::constructor().Env(), value));
}

TO_JS_IMPL(RTCRtpSender*, value) {
  return Pure(napi::UnsafeToV8(value->Value()));
}

}  // namespace node_webrtc
