/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_dtls_transport.h"

#include <memory>
#include <type_traits>
#include <utility>

#include <node-addon-api/napi.h>
#include <webrtc/api/peer_connection_interface.h>  // IWYU pragma: keep
#include <webrtc/api/rtc_error.h>
#include <webrtc/rtc_base/location.h>
#include <webrtc/rtc_base/thread.h>

#include "src/converters.h"
#include "src/converters/napi.h"
#include "src/dictionaries/node_webrtc/some_error.h"
#include "src/enums/webrtc/dtls_transport_state.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"
#include "src/node/events.h"

namespace node_webrtc {

Napi::FunctionReference& RTCDtlsTransport::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCDtlsTransport::RTCDtlsTransport(const Napi::CallbackInfo& info)
  : napi::AsyncObjectWrapWithLoop<RTCDtlsTransport>("RTCDtlsTransport", *this, info) {
  if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct an RTCDtlsTransport").ThrowAsJavaScriptException();
    return;
  }

  auto factory = PeerConnectionFactory::Unwrap(info[0].ToObject());
  auto transport = *info[1].As<Napi::External<rtc::scoped_refptr<webrtc::DtlsTransportInterface>>>().Data();

  _factory = factory;
  _factory->Ref();

  _transport = std::move(transport);

  _factory->_workerThread->Invoke<void>(RTC_FROM_HERE, [this]() {
    _transport->RegisterObserver(this);
    _state = _transport->Information().state();
    if (_state == webrtc::DtlsTransportState::kClosed) {
      Stop();
    }
  });
}

RTCDtlsTransport::~RTCDtlsTransport() {
  _factory->Unref();
  _factory = nullptr;
}  // NOLINT

void RTCDtlsTransport::Stop() {
  _transport->UnregisterObserver();
  napi::AsyncObjectWrapWithLoop<RTCDtlsTransport>::Stop();
}

void RTCDtlsTransport::OnStateChange(webrtc::DtlsTransportInformation information) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _state = information.state();
  }

  Dispatch(CreateCallback<RTCDtlsTransport>([this]() {
    auto env = Env();
    Napi::HandleScope scope(env);
    auto event = Napi::Object::New(env);
    event.Set("type", Napi::String::New(env, "statechange"));
    MakeCallback("dispatchEvent", { event });
  }));

  if (information.state() == webrtc::DtlsTransportState::kClosed) {
    Stop();
  }
}

void RTCDtlsTransport::OnError(webrtc::RTCError rtcError) {
  auto maybeError = From<SomeError>(&rtcError);
  if (maybeError.IsValid()) {
    auto error = maybeError.UnsafeFromValid();
    Dispatch(CreateCallback<RTCDtlsTransport>([this, error]() {
      auto env = Env();
      Napi::HandleScope scope(env);
      auto maybeValue = From<Napi::Value>(std::make_pair(env, error));
      if (maybeValue.IsValid()) {
        auto value = maybeValue.UnsafeFromValid();
        auto event = Napi::Object::New(env);
        event.Set("type", Napi::String::New(env, "error"));
        event.Set("error", value);
        MakeCallback("dispatchEvent", { event });
      }
    }));
  }
}

Napi::Value RTCDtlsTransport::GetState(const Napi::CallbackInfo& info) {
  std::lock_guard<std::mutex> lock(_mutex);
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _state, result, Napi::Value)
  return result;
}

Wrap <
RTCDtlsTransport*,
rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
PeerConnectionFactory*
> * RTCDtlsTransport::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCDtlsTransport*,
  rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
  PeerConnectionFactory*
  > (RTCDtlsTransport::Create);
  return wrap;
}

RTCDtlsTransport* RTCDtlsTransport::Create(
    PeerConnectionFactory* factory,
    rtc::scoped_refptr<webrtc::DtlsTransportInterface> transport) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto object = constructor().New({
    factory->Value(),
    Napi::External<rtc::scoped_refptr<webrtc::DtlsTransportInterface>>::New(env, &transport)
  });

  return RTCDtlsTransport::Unwrap(object);
}

void RTCDtlsTransport::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "RTCDtlsTransport", {
    InstanceAccessor("state", &RTCDtlsTransport::GetState, nullptr)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCDtlsTransport", func);
}

}  // namespace node_webrtc
