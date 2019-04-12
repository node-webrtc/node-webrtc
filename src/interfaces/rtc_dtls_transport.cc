/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_dtls_transport.h"

#include <type_traits>
#include <utility>

#include <nan.h>
#include <v8.h>
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
  if (info.Length() != 2 || !info[0].IsExternal() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct an RTCDtlsTransport").ThrowAsJavaScriptException();
    return;
  }

  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[0]))->Value());
  auto transport = *static_cast<rtc::scoped_refptr<webrtc::DtlsTransportInterface>*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[1]))->Value());

  _factory = std::move(factory);
  _transport = std::move(transport);

  _factory->_workerThread->Invoke<void>(RTC_FROM_HERE, [this]() {
    _transport->RegisterObserver(this);
    _state = _transport->Information().state();
    if (_state == webrtc::DtlsTransportState::kClosed) {
      Stop();
    }
  });
}

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
std::shared_ptr<PeerConnectionFactory>
> * RTCDtlsTransport::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCDtlsTransport*,
  rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
  std::shared_ptr<PeerConnectionFactory>
  > (RTCDtlsTransport::Create);
  return wrap;
}

RTCDtlsTransport* RTCDtlsTransport::Create(
    std::shared_ptr<PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::DtlsTransportInterface> transport) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto factoryExternal = Nan::New<v8::External>(static_cast<void*>(&factory));
  auto transportExternal = Nan::New<v8::External>(static_cast<void*>(&transport));

  auto object = constructor().New({
    napi::UnsafeFromV8(env, factoryExternal),
    napi::UnsafeFromV8(env, transportExternal)
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
