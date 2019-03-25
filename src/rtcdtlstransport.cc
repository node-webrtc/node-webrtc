/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcdtlstransport.h"

#include "src/converters.h"
#include "src/converters/enums.h"
#include "src/peerconnectionfactory.h"

Nan::Persistent<v8::Function>& node_webrtc::RTCDtlsTransport::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

Nan::Persistent<v8::FunctionTemplate>& node_webrtc::RTCDtlsTransport::tpl() {
  static Nan::Persistent<v8::FunctionTemplate> tpl;
  return tpl;
}

node_webrtc::RTCDtlsTransport::RTCDtlsTransport(
    std::shared_ptr<node_webrtc::PeerConnectionFactory> factory
    , rtc::scoped_refptr<webrtc::DtlsTransportInterface> transport)
  : node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCDtlsTransport>("RTCDtlsTransport", *this)
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

void node_webrtc::RTCDtlsTransport::Stop() {
  _transport->UnregisterObserver();
  node_webrtc::AsyncObjectWrapWithLoop<RTCDtlsTransport>::Stop();
}

void node_webrtc::RTCDtlsTransport::OnStateChange(webrtc::DtlsTransportInformation information) {
  {
    std::lock_guard<std::mutex> lock(_mutex);
    _state = information.state();
  }

  Dispatch(node_webrtc::Callback<node_webrtc::RTCDtlsTransport>::Create([this]() {
    Nan::HandleScope scope;
    auto event = Nan::New<v8::Object>();
    Nan::Set(event, Nan::New("type").ToLocalChecked(), Nan::New("statechange").ToLocalChecked());
    MakeCallback("dispatchEvent", 1, reinterpret_cast<v8::Local<v8::Value>*>(&event));
  }));

  if (information.state() == webrtc::DtlsTransportState::kClosed) {
    Stop();
  }
}

void node_webrtc::RTCDtlsTransport::OnError(webrtc::RTCError rtcError) {
  auto maybeError = node_webrtc::From<node_webrtc::SomeError>(&rtcError);
  if (maybeError.IsValid()) {
    auto error = maybeError.UnsafeFromValid();
    Dispatch(node_webrtc::Callback<node_webrtc::RTCDtlsTransport>::Create([this, error]() {
      Nan::HandleScope scope;
      auto maybeValue = node_webrtc::From<v8::Local<v8::Value>>(error);
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

NAN_METHOD(node_webrtc::RTCDtlsTransport::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct an RTCDtlsTransport");
  }
  auto factory = *static_cast<std::shared_ptr<node_webrtc::PeerConnectionFactory>*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto transport = *static_cast<rtc::scoped_refptr<webrtc::DtlsTransportInterface>*>(v8::Local<v8::External>::Cast(info[1])->Value());
  auto object = new node_webrtc::RTCDtlsTransport(std::move(factory), std::move(transport));
  object->Wrap(info.This());
  info.GetReturnValue().Set(info.This());
}

NAN_GETTER(node_webrtc::RTCDtlsTransport::GetState) {
  (void) property;
  auto self = AsyncObjectWrapWithLoop<node_webrtc::RTCDtlsTransport>::Unwrap(info.Holder());
  std::lock_guard<std::mutex> lock(self->_mutex);
  auto state = self->_state;
  CONVERT_OR_THROW_AND_RETURN(state, result, v8::Local<v8::Value>);
  info.GetReturnValue().Set(result);
}

node_webrtc::Wrap <
node_webrtc::RTCDtlsTransport*,
rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
std::shared_ptr<node_webrtc::PeerConnectionFactory>
> * node_webrtc::RTCDtlsTransport::wrap() {
  static auto wrap = new node_webrtc::Wrap <
  node_webrtc::RTCDtlsTransport*,
  rtc::scoped_refptr<webrtc::DtlsTransportInterface>,
  std::shared_ptr<node_webrtc::PeerConnectionFactory>
  > (node_webrtc::RTCDtlsTransport::Create);
  return wrap;
}

node_webrtc::RTCDtlsTransport* node_webrtc::RTCDtlsTransport::Create(
    std::shared_ptr<node_webrtc::PeerConnectionFactory> factory,
    rtc::scoped_refptr<webrtc::DtlsTransportInterface> transport) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&factory));
  cargv[1] = Nan::New<v8::External>(static_cast<void*>(&transport));
  auto object = Nan::NewInstance(Nan::New(node_webrtc::RTCDtlsTransport::constructor()), 2, cargv).ToLocalChecked();
  return AsyncObjectWrapWithLoop<node_webrtc::RTCDtlsTransport>::Unwrap(object);
}

void node_webrtc::RTCDtlsTransport::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  node_webrtc::RTCDtlsTransport::tpl().Reset(tpl);
  tpl->SetClassName(Nan::New("RTCDtlsTransport").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("state").ToLocalChecked(), GetState, nullptr);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCDtlsTransport").ToLocalChecked(), tpl->GetFunction());
}
