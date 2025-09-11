/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_dtls_transport.hh"

#include <memory>
#include <type_traits>
#include <utility>

#include <node-addon-api/napi.h>
#include <webrtc/api/peer_connection_interface.h> // IWYU pragma: keep
#include <webrtc/api/rtc_error.h>
#include <webrtc/rtc_base/location.h>
#include <webrtc/rtc_base/thread.h>

#include "src/converters.hh"
#include "src/converters/napi.hh"
#include "src/converters/webrtc.hh"
#include "src/dictionaries/node_webrtc/some_error.hh"
#include "src/enums/webrtc/dtls_transport_state.hh" // IWYU pragma: keep
#include "src/functional/validation.hh"
#include "src/interfaces/rtc_ice_transport.hh"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.hh"
#include "src/node/events.hh"

namespace node_webrtc {

Napi::FunctionReference &RTCDtlsTransport::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

namespace {
std::vector<rtc::Buffer>
copy_certs(webrtc::DtlsTransportInformation const &information) {
  auto certs = information.remote_ssl_certificates();
  if (certs) {
    auto size = certs->GetSize();
    auto ders = std::vector<rtc::Buffer>();
    ders.reserve(size);
    for (unsigned long i = 0; i < size; i++) {
      // TODO(mroberts): I don't know a more approriate value to initialize
      // with.
      auto buffer = rtc::Buffer(1);
      certs->Get(i).ToDER(&buffer);
      ders.emplace_back(std::move(buffer));
    }
    return ders;
  }

  return {};
}
} // namespace

RTCDtlsTransport::RTCDtlsTransport(const Napi::CallbackInfo &info)
    : AsyncObjectWrapWithLoop<RTCDtlsTransport>("RTCDtlsTransport", *this,
                                                info) {
  if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct an RTCDtlsTransport")
        .ThrowAsJavaScriptException();
    return;
  }

  _factory = PeerConnectionFactory::Unwrap(info[0].ToObject());
  auto transport =
      *info[1]
           .As<Napi::External<
               rtc::scoped_refptr<webrtc::DtlsTransportInterface>>>()
           .Data();

  _transport = std::move(transport);

  // NOTE(mroberts): Ensure we create this.
  _transport_wrap.GetOrCreate(_factory, _transport->ice_transport());

  _factory->WorkerThread()->Invoke<void>(RTC_FROM_HERE, [this]() {
    _transport->RegisterObserver(this);
    auto information = _transport->Information();
    _state = information.state();
    _certs = copy_certs(information);
    if (_state == webrtc::DtlsTransportState::kClosed) {
      Stop();
    }
  });
}

RTCDtlsTransport::~RTCDtlsTransport() {
  Napi::HandleScope scope(PeerConnectionFactory::constructor().Env());

  wrap()->Release(this);
}

void RTCDtlsTransport::Stop() {
  _transport->UnregisterObserver();
  auto ice_transport = _transport_wrap.Get(_transport->ice_transport());
  if (ice_transport) {
    ice_transport->OnRTCDtlsTransportStopped();
  }
  AsyncObjectWrapWithLoop<RTCDtlsTransport>::Stop();
}

void RTCDtlsTransport::OnStateChange(
    webrtc::DtlsTransportInformation information) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _state = information.state();
    _certs = copy_certs(information);
  }

  Dispatch(CreateCallback<RTCDtlsTransport>([this]() {
    auto env = Env();
    Napi::HandleScope scope(env);
    auto event = Napi::Object::New(env);
    event.Set("type", Napi::String::New(env, "statechange"));
    MakeCallback("dispatchEvent", {event});
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
        MakeCallback("dispatchEvent", {event});
      }
    }));
  }
}

Napi::Value RTCDtlsTransport::GetIceTransport(const Napi::CallbackInfo &) {
  return _transport_wrap.GetOrCreate(_factory, _transport->ice_transport())
      ->Value();
}

Napi::Value RTCDtlsTransport::GetState(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(_mutex);
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _state, result, Napi::Value)
  return result;
}

Napi::Value
RTCDtlsTransport::GetRemoteCertificates(const Napi::CallbackInfo &info) {
  std::lock_guard<std::mutex> lock(_mutex);
  auto certs = std::vector<rtc::Buffer *>();
  certs.reserve(_certs.size());
  for (auto &cert : _certs) {
    certs.push_back(&cert);
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), certs, result, Napi::Value)
  return result;
}

Wrap<RTCDtlsTransport *, rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
     PeerConnectionFactory *> *
RTCDtlsTransport::wrap() {
  static auto wrap =
      new node_webrtc::Wrap<RTCDtlsTransport *,
                            rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
                            PeerConnectionFactory *>(RTCDtlsTransport::Create);
  return wrap;
}

RTCDtlsTransport *RTCDtlsTransport::Create(
    PeerConnectionFactory *factory,
    rtc::scoped_refptr<webrtc::DtlsTransportInterface> transport) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto object = constructor().New(
      {factory->Value(),
       Napi::External<rtc::scoped_refptr<webrtc::DtlsTransportInterface>>::New(
           env, &transport)});

  auto unwrapped = Unwrap(object);
  return unwrapped;
}

void RTCDtlsTransport::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(
      env, "RTCDtlsTransport",
      {InstanceMethod("getRemoteCertificates",
                      &RTCDtlsTransport::GetRemoteCertificates),
       InstanceAccessor("iceTransport", &RTCDtlsTransport::GetIceTransport,
                        nullptr),
       InstanceAccessor("state", &RTCDtlsTransport::GetState, nullptr)});

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCDtlsTransport", func);
}

} // namespace node_webrtc
