/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_rtp_receiver.h"

#include <webrtc/api/rtp_receiver_interface.h>

#include "src/converters.h"
#include "src/converters/interfaces.h"
#include "src/converters/napi.h"
#include "src/converters/v8.h"
#include "src/dictionaries/webrtc/rtp_parameters.h"
#include "src/dictionaries/webrtc/rtp_source.h"
#include "src/interfaces/media_stream_track.h"
#include "src/interfaces/rtc_dtls_transport.h"
#include "src/node/utility.h"

namespace node_webrtc {

Napi::FunctionReference& RTCRtpReceiver::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCRtpReceiver::RTCRtpReceiver(const Napi::CallbackInfo& info)
  : napi::AsyncObjectWrap<RTCRtpReceiver>("RTCRtpReceiver", info) {
  if (info.Length() != 2 || !info[0].IsExternal() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct a RTCRtpReceiver").ThrowAsJavaScriptException();
    return;
  }

  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[0]))->Value());
  auto receiver = *static_cast<rtc::scoped_refptr<webrtc::RtpReceiverInterface>*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[1]))->Value());

  _factory = std::move(factory);
  _receiver = std::move(receiver);
}

RTCRtpReceiver::~RTCRtpReceiver() {
  wrap()->Release(this);
}

Napi::Value RTCRtpReceiver::GetTrack(const Napi::CallbackInfo& info) {
  return napi::UnsafeFromV8(info.Env(), MediaStreamTrack::wrap()->GetOrCreate(_factory, _receiver->track())->ToObject());
}

Napi::Value RTCRtpReceiver::GetTransport(const Napi::CallbackInfo& info) {
  auto transport = _receiver->dtls_transport();
  return transport
      ? RTCDtlsTransport::wrap()->GetOrCreate(_factory, transport)->Value()
      : info.Env().Null();
}

Napi::Value RTCRtpReceiver::GetRtcpTransport(const Napi::CallbackInfo& info) {
  return info.Env().Null();
}

Napi::Value RTCRtpReceiver::GetCapabilities(const Napi::CallbackInfo& info) {
  Napi::Error::New(info.Env(), "Not yet implemented; file a feature request against node-webrtc").ThrowAsJavaScriptException();
  return info.Env().Undefined();
}

Napi::Value RTCRtpReceiver::GetParameters(const Napi::CallbackInfo& info) {
  auto parameters = _receiver->GetParameters();
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), parameters, result, Napi::Value)
  return result;
}

Napi::Value RTCRtpReceiver::GetContributingSources(const Napi::CallbackInfo& info) {
  auto contributingSources = std::vector<webrtc::RtpSource>();
  auto sources = _receiver->GetSources();
  for (const auto& source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::CSRC) {
      contributingSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), contributingSources, result, Napi::Value)
  return result;
}

Napi::Value RTCRtpReceiver::GetSynchronizationSources(const Napi::CallbackInfo& info) {
  auto synchronizationSources = std::vector<webrtc::RtpSource>();
  auto sources = _receiver->GetSources();
  for (const auto& source : sources) {
    if (source.source_type() == webrtc::RtpSourceType::SSRC) {
      synchronizationSources.push_back(source);
    }
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), synchronizationSources, result, Napi::Value)
  return result;
}

Napi::Value RTCRtpReceiver::GetStats(const Napi::CallbackInfo& info) {
  CREATE_DEFERRED(info.Env(), deferred)
  napi::Reject(deferred, Napi::Error::New(info.Env(), "Not yet implemented; file a feature request against node-webrtc"));
  return deferred.Promise();
}

Wrap <
RTCRtpReceiver*,
rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
std::shared_ptr<PeerConnectionFactory>
> * RTCRtpReceiver::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCRtpReceiver*,
  rtc::scoped_refptr<webrtc::RtpReceiverInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > (RTCRtpReceiver::Create);
  return wrap;
}

RTCRtpReceiver* RTCRtpReceiver::Create(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto factoryExternal = Nan::New<v8::External>(static_cast<void*>(&factory));
  auto receiverExternal = Nan::New<v8::External>(static_cast<void*>(&receiver));

  auto object = constructor().New({
    napi::UnsafeFromV8(env, factoryExternal),
    napi::UnsafeFromV8(env, receiverExternal)
  });

  return RTCRtpReceiver::Unwrap(object);
}

void RTCRtpReceiver::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "RTCRtpReceiver", {
    InstanceAccessor("track", &RTCRtpReceiver::GetTrack, nullptr),
    InstanceAccessor("transport", &RTCRtpReceiver::GetTransport, nullptr),
    InstanceAccessor("rtcpTransport", &RTCRtpReceiver::GetRtcpTransport, nullptr),
    InstanceMethod("getParameters", &RTCRtpReceiver::GetParameters),
    InstanceMethod("getContributingSources", &RTCRtpReceiver::GetContributingSources),
    InstanceMethod("getSynchronizationSources", &RTCRtpReceiver::GetSynchronizationSources),
    InstanceMethod("getStats", &RTCRtpReceiver::GetStats),
    StaticMethod("getCapabilities", &RTCRtpReceiver::GetCapabilities)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCRtpReceiver", func);
}

FROM_NAPI_IMPL(RTCRtpReceiver*, value) {
  return From<Napi::Object>(value).FlatMap<RTCRtpReceiver*>([](Napi::Object object) {
    auto isRTCRtpReceiver = false;
    napi_instanceof(object.Env(), object, RTCRtpReceiver::constructor().Value(), &isRTCRtpReceiver);
    if (object.Env().IsExceptionPending()) {
      return Validation<RTCRtpReceiver*>::Invalid(object.Env().GetAndClearPendingException().Message());
    } else if (!isRTCRtpReceiver) {
      return Validation<RTCRtpReceiver*>::Invalid("This is not an instance of RTCRtpReceiver");
    }
    return Pure(RTCRtpReceiver::Unwrap(object));
  });
}

TO_NAPI_IMPL(RTCRtpReceiver*, pair) {
  return Pure(pair.second->Value().As<Napi::Value>());
}

TO_JS_IMPL(RTCRtpReceiver*, value) {
  return Pure(napi::UnsafeToV8(value->Value()));
}

}  // namespace node_webrtc
