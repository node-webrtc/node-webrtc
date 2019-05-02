/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_ice_transport.h"

#include "src/converters/arguments.h"
#include "src/enums/webrtc/ice_connection_state.h"
#include "src/enums/webrtc/ice_gathering_state.h"
#include "src/enums/webrtc/ice_role.h"
#include "src/enums/webrtc/ice_transport_state.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"

namespace node_webrtc {

Napi::FunctionReference& RTCIceTransport::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCIceTransport::RTCIceTransport(const Napi::CallbackInfo& info)
  : AsyncObjectWrapWithLoop<RTCIceTransport>("RTCIceTransport", *this, info) {
  if (info.Length() != 2 || !info[0].IsObject() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct an RTCIceTransport").ThrowAsJavaScriptException();
    return;
  }

  auto factory = PeerConnectionFactory::Unwrap(info[0].ToObject());
  auto transport = *info[1].As<Napi::External<rtc::scoped_refptr<webrtc::IceTransportInterface>>>().Data();

  _factory = factory;
  _factory->Ref();

  _transport = std::move(transport);

  _factory->_workerThread->Invoke<void>(RTC_FROM_HERE, [this]() {
    auto internal = _transport->internal();
    internal->SignalIceTransportStateChanged.connect(this, &RTCIceTransport::OnStateChanged);
    internal->SignalGatheringState.connect(this, &RTCIceTransport::OnGatheringStateChanged);
    TakeSnapshot();
    if (_state == webrtc::IceTransportState::kClosed) {
      Stop();
    }
  });
}

void RTCIceTransport::TakeSnapshot() {
  std::lock_guard<std::mutex> lock(_mutex);
  auto internal = _transport->internal();
  _role = internal->GetIceRole();
  _component = internal->component() == 1 ? RTCIceComponent::kRtp : RTCIceComponent::kRtcp;
  _state = internal->GetIceTransportState();
  _gathering_state = internal->gathering_state();
}

RTCIceTransport::~RTCIceTransport() {
  Napi::HandleScope scope(PeerConnectionFactory::constructor().Env());
  _factory->Unref();
  _factory = nullptr;
}  // NOLINT

void RTCIceTransport::OnRTCDtlsTransportStopped() {
  std::lock_guard<std::mutex> lock(_mutex);
  _state = webrtc::IceTransportState::kClosed;
  _gathering_state = cricket::IceGatheringState::kIceGatheringComplete;
  Stop();
}

void RTCIceTransport::Stop() {
  // _factory->_workerThread->Invoke<void>(RTC_FROM_HERE, [this]() {
  //   _transport->internal()->SignalIceTransportStateChanged.disconnect(this);
  //   _transport->internal()->SignalGatheringState.disconnect(this);
  // });
  AsyncObjectWrapWithLoop<RTCIceTransport>::Stop();
}

Wrap <
RTCIceTransport*,
rtc::scoped_refptr<webrtc::IceTransportInterface>,
PeerConnectionFactory*
> * RTCIceTransport::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  RTCIceTransport*,
  rtc::scoped_refptr<webrtc::IceTransportInterface>,
  PeerConnectionFactory*
  > (RTCIceTransport::Create);
  return wrap;
}

RTCIceTransport* RTCIceTransport::Create(
    PeerConnectionFactory* factory,
    rtc::scoped_refptr<webrtc::IceTransportInterface> transport) {
  auto env = constructor().Env();
  Napi::HandleScope scope(env);

  auto object = constructor().New({
    factory->Value(),
    Napi::External<rtc::scoped_refptr<webrtc::IceTransportInterface>>::New(env, &transport)
  });

  return RTCIceTransport::Unwrap(object);
}

void RTCIceTransport::OnStateChanged(cricket::IceTransportInternal*) {
  TakeSnapshot();

  Dispatch(CreateCallback<RTCIceTransport>([this]() {
    auto env = Env();
    Napi::HandleScope scope(env);
    auto event = Napi::Object::New(env);
    event.Set("type", Napi::String::New(env, "statechange"));
    MakeCallback("dispatchEvent", { event });
  }));

  if (_state == webrtc::IceTransportState::kClosed) {
    Stop();
  }
}

void RTCIceTransport::OnGatheringStateChanged(cricket::IceTransportInternal*) {
  TakeSnapshot();

  Dispatch(CreateCallback<RTCIceTransport>([this]() {
    auto env = Env();
    Napi::HandleScope scope(env);
    auto event = Napi::Object::New(env);
    event.Set("type", Napi::String::New(env, "gatheringstatechange"));
    MakeCallback("dispatchEvent", { event });
  }));
}

Napi::Value RTCIceTransport::GetRole(const Napi::CallbackInfo& info) {
  std::lock_guard<std::mutex> lock(_mutex);
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _role, result, Napi::Value)
  return result;
}

Napi::Value RTCIceTransport::GetComponent(const Napi::CallbackInfo& info) {
  std::lock_guard<std::mutex> lock(_mutex);
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _component, result, Napi::Value)
  return result;
}

Napi::Value RTCIceTransport::GetState(const Napi::CallbackInfo& info) {
  std::lock_guard<std::mutex> lock(_mutex);
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _state, result, Napi::Value)
  return result;
}

Napi::Value RTCIceTransport::GetGatheringState(const Napi::CallbackInfo& info) {
  std::lock_guard<std::mutex> lock(_mutex);
  webrtc::PeerConnectionInterface::IceGatheringState state;
  switch (_gathering_state) {
    case cricket::IceGatheringState::kIceGatheringNew:
      state = webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete;
      break;
    case cricket::IceGatheringState::kIceGatheringGathering:
      state = webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete;
      break;
    case cricket::IceGatheringState::kIceGatheringComplete:
      state = webrtc::PeerConnectionInterface::IceGatheringState::kIceGatheringComplete;
      break;
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), state, result, Napi::Value)
  return result;
}

Napi::Value RTCIceTransport::GetLocalCandidates(const Napi::CallbackInfo& info) {
  Napi::Error::New(info.Env(), "Not yet implemented!").ThrowAsJavaScriptException();
  return info.Env().Undefined();
}

Napi::Value RTCIceTransport::GetRemoteCandidates(const Napi::CallbackInfo& info) {
  Napi::Error::New(info.Env(), "Not yet implemented!").ThrowAsJavaScriptException();
  return info.Env().Undefined();
}

Napi::Value RTCIceTransport::GetSelectedCandidatePair(const Napi::CallbackInfo& info) {
  Napi::Error::New(info.Env(), "Not yet implemented!").ThrowAsJavaScriptException();
  return info.Env().Undefined();
}

Napi::Value RTCIceTransport::GetLocalParameters(const Napi::CallbackInfo& info) {
  Napi::Error::New(info.Env(), "Not yet implemented!").ThrowAsJavaScriptException();
  return info.Env().Undefined();
}

Napi::Value RTCIceTransport::GetRemoteParameters(const Napi::CallbackInfo& info) {
  Napi::Error::New(info.Env(), "Not yet implemented!").ThrowAsJavaScriptException();
  return info.Env().Undefined();
}

void RTCIceTransport::Init(Napi::Env env, Napi::Object exports) {
  auto func = DefineClass(env, "RTCIceTransport", {
    InstanceAccessor("role", &RTCIceTransport::GetRole, nullptr),
    InstanceAccessor("component", &RTCIceTransport::GetComponent, nullptr),
    InstanceAccessor("state", &RTCIceTransport::GetState, nullptr),
    InstanceAccessor("gatheringState", &RTCIceTransport::GetGatheringState, nullptr),
    InstanceMethod("getLocalCandidates", &RTCIceTransport::GetLocalCandidates),
    InstanceMethod("getRemoteCandidates", &RTCIceTransport::GetRemoteCandidates),
    InstanceMethod("getSelectedCandidatePair", &RTCIceTransport::GetSelectedCandidatePair),
    InstanceMethod("getLocalParameters", &RTCIceTransport::GetLocalParameters),
    InstanceMethod("getRemoteParameters", &RTCIceTransport::GetRemoteParameters)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCIceTransport", func);
}

}  // namespace node_webrtc
