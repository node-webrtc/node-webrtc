/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_dtls_transport.h"

#include <type_traits>

#include <webrtc/api/peer_connection_interface.h>  // IWYU pragma: keep
#include <webrtc/api/rtc_error.h>
#include <webrtc/rtc_base/location.h>
#include <webrtc/rtc_base/thread.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/dictionaries/node_webrtc/some_error.h"
#include "src/enums/webrtc/dtls_transport_state.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/interfaces/rtc_peer_connection/peer_connection_factory.h"
#include "src/node/events.h"

namespace node_webrtc {

Nan::Persistent<v8::Function>& RTCDtlsTransport::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& RTCDtlsTransport::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

RTCDtlsTransport::RTCDtlsTransport(
    std::shared_ptr<PeerConnectionFactory> factory
    , rtc::scoped_refptr<webrtc::DtlsTransportInterface> transport)
  : AsyncObjectWrapWithLoop<RTCDtlsTransport>("RTCDtlsTransport", *this)
  , _factory(std::move(factory))
  , _transport(std::move(transport)) {
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
  AsyncObjectWrapWithLoop<RTCDtlsTransport>::Stop();
}

void RTCDtlsTransport::OnStateChange(webrtc::DtlsTransportInformation information) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _state = information.state();
  }

  Dispatch(CreateCallback<RTCDtlsTransport>([this]() {
    Nan::HandleScope scope;
    auto event = Nan::New<v8::Object>();
    Nan::Set(event, Nan::New("type").ToLocalChecked(), Nan::New("statechange").ToLocalChecked());
    MakeCallback("dispatchEvent", 1, reinterpret_cast<v8::Local<v8::Value>*>(&event));
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
      Nan::HandleScope scope;
      auto maybeValue = From<v8::Local<v8::Value>>(error);
      if (maybeValue.IsValid()) {
        auto value = maybeValue.UnsafeFromValid();
        auto event = Nan::New<v8::Object>();
        Nan::Set(event, Nan::New("type").ToLocalChecked(), Nan::New("error").ToLocalChecked());
        Nan::Set(event, Nan::New("error").ToLocalChecked(), value);
        MakeCallback("dispatchEvent", 1, reinterpret_cast<v8::Local<v8::Value> *>(&event));
      }
    }));
  }
}

NAN_METHOD(RTCDtlsTransport::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct an RTCDtlsTransport");
  }
  auto factory = *static_cast<std::shared_ptr<PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto transport = *static_cast<rtc::scoped_refptr<webrtc::DtlsTransportInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());
  auto object = new RTCDtlsTransport(std::move(factory), std::move(transport));
  object->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(RTCDtlsTransport::GetState) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<RTCDtlsTransport>::Unwrap(info.Holder());
  std::lock_guard<std::mutex> lock(self->_mutex);
  auto state = self->_state;
  CONVERT_OR_THROW_AND_RETURN(state, result, v8::Local<v8::Value>)
  info.GetReturnValue().Set(result);
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
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&transport));
  auto object = Nan::NewInstance(Nan::New(RTCDtlsTransport::constructor()), 2, cargv).ToLocalChecked();
  return AsyncObjectWrapWithLoop<RTCDtlsTransport>::Unwrap(object);
}

void RTCDtlsTransport::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  RTCDtlsTransport::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCDtlsTransport").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("state").ToLocalChecked(), GetState, nullptr);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCDtlsTransport").ToLocalChecked(), tpl->GetFunction());
}

}  // namespace node_webrtc
