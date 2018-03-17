/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peerconnection.h"

#include "webrtc/base/refcountedobject.h"

#include "common.h"
#include "converters/arguments.h"
#include "converters/webrtc.h"
#include "create-answer-observer.h"
#include "create-offer-observer.h"
#include "datachannel.h"
#include "functional/maybe.h"
#include "peerconnectionfactory.h"
#include "rtcstatsresponse.h"
#include "set-local-description-observer.h"
#include "set-remote-description-observer.h"
#include "stats-observer.h"

using node_webrtc::From;
using node_webrtc::Maybe;
using node_webrtc::PeerConnection;
using node_webrtc::PeerConnectionFactory;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Integer;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Value;
using v8::Array;
using webrtc::DataChannelInit;
using webrtc::IceCandidateInterface;
using webrtc::SessionDescriptionInterface;

using IceConnectionState = webrtc::PeerConnectionInterface::IceConnectionState;
using IceGatheringState = webrtc::PeerConnectionInterface::IceGatheringState;
using RTCConfiguration = webrtc::PeerConnectionInterface::RTCConfiguration;
using SignalingState = webrtc::PeerConnectionInterface::SignalingState;

Nan::Persistent<Function> PeerConnection::constructor;

//
// PeerConnection
//

PeerConnection::PeerConnection(RTCConfiguration configuration)
  : loop(uv_default_loop()) {
  _createOfferObserver = new rtc::RefCountedObject<CreateOfferObserver>(this);
  _createAnswerObserver = new rtc::RefCountedObject<CreateAnswerObserver>(this);
  _setLocalDescriptionObserver = new rtc::RefCountedObject<SetLocalDescriptionObserver>(this);
  _setRemoteDescriptionObserver = new rtc::RefCountedObject<SetRemoteDescriptionObserver>(this);

  // TODO(mroberts): Read `factory` (non-standard) from RTCConfiguration?
  _factory = PeerConnectionFactory::GetOrCreateDefault();
  _shouldReleaseFactory = true;

  _jinglePeerConnection = _factory->factory()->CreatePeerConnection(configuration, nullptr, nullptr, nullptr, this);

  uv_mutex_init(&lock);
  uv_async_init(loop, &async, reinterpret_cast<uv_async_cb>(Run));

  async.data = this;
}

PeerConnection::~PeerConnection() {
  TRACE_CALL;
  _jinglePeerConnection = nullptr;
  if (_factory) {
    if (_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    _factory = nullptr;
  }
  uv_mutex_destroy(&lock);
  TRACE_END;
}

void PeerConnection::QueueEvent(AsyncEventType type, void* data) {
  TRACE_CALL;
  AsyncEvent evt;
  evt.type = type;
  evt.data = data;
  uv_mutex_lock(&lock);
  _events.push(evt);
  uv_mutex_unlock(&lock);

  uv_async_send(&async);
  TRACE_END;
}

void PeerConnection::Run(uv_async_t* handle, int status) {
  Nan::HandleScope scope;

  auto self = static_cast<PeerConnection*>(handle->data);
  TRACE_CALL_P((uintptr_t)self);
  auto do_shutdown = false;

  while (true) {
    auto pc = self->handle();

    uv_mutex_lock(&self->lock);
    bool empty = self->_events.empty();
    if (empty) {
      uv_mutex_unlock(&self->lock);
      break;
    }
    AsyncEvent evt = self->_events.front();
    self->_events.pop();
    uv_mutex_unlock(&self->lock);

    TRACE_U("evt.type", evt.type);
    if (PeerConnection::ERROR_EVENT & evt.type) {
      PeerConnection::ErrorEvent* data = static_cast<PeerConnection::ErrorEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onerror").ToLocalChecked()));
      Local<Value> argv[1];
      argv[0] = Nan::Error(data->msg.c_str());
      Nan::MakeCallback(pc, callback, 1, argv);
    } else if (PeerConnection::SDP_EVENT & evt.type) {
      PeerConnection::SdpEvent* data = static_cast<PeerConnection::SdpEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onsuccess").ToLocalChecked()));
      Local<Value> argv[1];
      argv[0] = Nan::New(data->desc.c_str()).ToLocalChecked();
      Nan::MakeCallback(pc, callback, 1, argv);
    } else if (PeerConnection::GET_STATS_SUCCESS & evt.type) {
      PeerConnection::GetStatsEvent* data = static_cast<PeerConnection::GetStatsEvent*>(evt.data);
      Nan::Callback* callback = data->callback;
      Local<Value> cargv[1];
      cargv[0] = Nan::New<External>(static_cast<void*>(&data->reports));
      Local<Value> argv[1];
      argv[0] = Nan::New(RTCStatsResponse::constructor)->NewInstance(1, cargv);
      callback->Call(1, argv);
    } else if (PeerConnection::VOID_EVENT & evt.type) {
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onsuccess").ToLocalChecked()));
      Local<Value> argv[1];
      Nan::MakeCallback(pc, callback, 0, argv);
    } else if (PeerConnection::SIGNALING_STATE_CHANGE & evt.type) {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onsignalingstatechange").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Local<Value> argv[1];
        argv[0] = Nan::New<Uint32>(data->state);
        Nan::MakeCallback(pc, callback, 1, argv);
      }
      if (webrtc::PeerConnectionInterface::kClosed == data->state) {
        do_shutdown = true;
      }
    } else if (PeerConnection::ICE_CONNECTION_STATE_CHANGE & evt.type) {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("oniceconnectionstatechange").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Local<Value> argv[1];
        argv[0] = Nan::New<Uint32>(data->state);
        Nan::MakeCallback(pc, callback, 1, argv);
      }
    } else if (PeerConnection::ICE_GATHERING_STATE_CHANGE & evt.type) {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onicegatheringstatechange").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Local<Value> argv[1];
        argv[0] = Nan::New<Uint32>(data->state);
        Nan::MakeCallback(pc, callback, 1, argv);
      }
    } else if (PeerConnection::ICE_CANDIDATE & evt.type) {
      PeerConnection::IceEvent* data = static_cast<PeerConnection::IceEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onicecandidate").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Local<Value> argv[3];
        argv[0] = Nan::New(data->candidate.c_str()).ToLocalChecked();
        argv[1] = Nan::New(data->sdpMid.c_str()).ToLocalChecked();
        argv[2] = Nan::New<Integer>(data->sdpMLineIndex);
        Nan::MakeCallback(pc, callback, 3, argv);
      }
    } else if (PeerConnection::NOTIFY_DATA_CHANNEL & evt.type) {
      PeerConnection::DataChannelEvent* data = static_cast<PeerConnection::DataChannelEvent*>(evt.data);
      DataChannelObserver* observer = data->observer;
      Local<Value> cargv[1];
      cargv[0] = Nan::New<External>(static_cast<void*>(observer));
      Local<Value> dc = Nan::New(DataChannel::constructor)->NewInstance(1, cargv);

      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("ondatachannel").ToLocalChecked()));
      Local<Value> argv[1];
      argv[0] = dc;
      Nan::MakeCallback(pc, callback, 1, argv);
    }
  }

  if (do_shutdown) {
    self->async.data = nullptr;
    self->Unref();
    uv_close(reinterpret_cast<uv_handle_t*>(handle), nullptr);
  }

  TRACE_END;
}

void PeerConnection::OnError() {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::SIGNALING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_CONNECTION_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_GATHERING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  TRACE_CALL;
  PeerConnection::IceEvent* data = new PeerConnection::IceEvent(candidate);
  QueueEvent(PeerConnection::ICE_CANDIDATE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> jingle_data_channel) {
  TRACE_CALL;
  DataChannelObserver* observer = new DataChannelObserver(_factory, jingle_data_channel);
  PeerConnection::DataChannelEvent* data = new PeerConnection::DataChannelEvent(observer);
  QueueEvent(PeerConnection::NOTIFY_DATA_CHANNEL, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnAddStream(webrtc::MediaStreamInterface* stream) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRemoveStream(webrtc::MediaStreamInterface* stream) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRenegotiationNeeded() {
  TRACE_CALL;
  TRACE_END;
}

NAN_METHOD(PeerConnection::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the PeerConnection.");
  }

  auto maybeConfiguration = From<Maybe<RTCConfiguration>, Nan::NAN_METHOD_ARGS_TYPE>(info);
  if (maybeConfiguration.IsInvalid()) {
    auto error = maybeConfiguration.ToErrors()[0];
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked());
  }

  // Tell em whats up
  auto obj = new PeerConnection(maybeConfiguration.UnsafeFromValid().FromMaybe(RTCConfiguration()));
  obj->Wrap(info.This());
  obj->Ref();

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;

  // TODO: Actually use the options.
  auto validationOptions = From<Maybe<RTCOfferOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
  [](const Maybe<RTCOfferOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCOfferOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(validationOptions.ToErrors()[0]).ToLocalChecked());
    return;
  }

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateOffer(self->_createOfferObserver, nullptr);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;

  // TODO: Actually use the options.
  auto validationOptions = From<Maybe<RTCAnswerOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
  [](const Maybe<RTCAnswerOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCAnswerOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(validationOptions.ToErrors()[0]).ToLocalChecked());
    return;
  }

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateAnswer(self->_createAnswerObserver, nullptr);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;

  auto maybeDescription = From<SessionDescriptionInterface*, Nan::NAN_METHOD_ARGS_TYPE>(info);
  if (maybeDescription.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(maybeDescription.ToErrors()[0]).ToLocalChecked());
    return;
  }
  auto description = maybeDescription.UnsafeFromValid();

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->SetLocalDescription(self->_setLocalDescriptionObserver, description);
  } else {
    delete description;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetRemoteDescription) {
  TRACE_CALL;

  auto maybeDescription = From<SessionDescriptionInterface*, Nan::NAN_METHOD_ARGS_TYPE>(info);
  if (maybeDescription.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(maybeDescription.ToErrors()[0]).ToLocalChecked());
    return;
  }
  auto description = maybeDescription.UnsafeFromValid();

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->SetRemoteDescription(self->_setRemoteDescriptionObserver, description);
  } else {
    delete description;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::AddIceCandidate) {
  TRACE_CALL;

  auto maybeCandidate = From<IceCandidateInterface*, Nan::NAN_METHOD_ARGS_TYPE>(info);
  if (maybeCandidate.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(maybeCandidate.ToErrors()[0]).ToLocalChecked());
    return;
  }
  auto candidate = maybeCandidate.UnsafeFromValid();

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr && self->_jinglePeerConnection->AddIceCandidate(candidate)) {
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_SUCCESS, static_cast<void*>(nullptr));
  } else {
    delete candidate;
    std::string error = std::string("Failed to set ICE candidate");
    if (self->_jinglePeerConnection == nullptr) {
      error += ", no jingle peer connection";
    }
    error += ".";
    PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(error);
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_ERROR, static_cast<void*>(data));
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateDataChannel) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection == nullptr) {
    info.GetReturnValue().Set(Nan::Undefined());
    return;
  }

  auto maybeArgs = From<std::tuple<std::string, DataChannelInit>, Nan::NAN_METHOD_ARGS_TYPE>(info);
  if (maybeArgs.IsInvalid()) {
    TRACE_END;
    auto error = maybeArgs.ToErrors()[0];
    Nan::ThrowTypeError(Nan::New(error).ToLocalChecked());
    return;
  }
  auto args = maybeArgs.UnsafeFromValid();
  auto label = std::get<0>(args);
  auto dataChannelInit = std::get<1>(args);

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface =
      self->_jinglePeerConnection->CreateDataChannel(label, &dataChannelInit);

  DataChannelObserver* observer = new DataChannelObserver(self->_factory, data_channel_interface);

  Local<Value> cargv[1];
  cargv[0] = Nan::New<External>(static_cast<void*>(observer));
  Local<Value> dc = Nan::New(DataChannel::constructor)->NewInstance(1, cargv);

  TRACE_END;
  info.GetReturnValue().Set(dc);
}

NAN_METHOD(PeerConnection::GetStats) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  Nan::Callback* onSuccess = new Nan::Callback(info[0].As<Function>());
  Nan::Callback* onFailure = new Nan::Callback(info[1].As<Function>());
  rtc::scoped_refptr<StatsObserver> statsObserver =
      new rtc::RefCountedObject<StatsObserver>(self, onSuccess);

  if (self->_jinglePeerConnection == nullptr) {
    Local<Value> argv[] = {
      Nan::New("data channel is closed").ToLocalChecked()
    };
    onFailure->Call(1, argv);
  } else if (!self->_jinglePeerConnection->GetStats(statsObserver, nullptr,
          webrtc::PeerConnectionInterface::kStatsOutputLevelStandard)) {
    // TODO: Include error?
    Local<Value> argv[] = {
      Nan::Null()
    };
    onFailure->Call(1, argv);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::UpdateIce) {
  TRACE_CALL;
  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::Close) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->Close();
  }

  self->_jinglePeerConnection = nullptr;

  if (self->_factory) {
    if (self->_shouldReleaseFactory) {
      PeerConnectionFactory::Release();
    }
    self->_factory = nullptr;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(PeerConnection::GetLocalDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection) {
    if (auto _description = self->_jinglePeerConnection->local_description()) {
      std::string sdp;
      if (_description->ToString(&sdp)) {
        auto type = _description->type();
        Local<Object> description = Nan::New<Object>();
        Nan::Set(description, Nan::New("type").ToLocalChecked(), Nan::New(type).ToLocalChecked());
        Nan::Set(description, Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
        result = description;
      }
    }
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetRemoteDescription) {
  TRACE_CALL;

  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection) {
    if (auto _description = self->_jinglePeerConnection->remote_description()) {
      std::string sdp;
      if (_description->ToString(&sdp)) {
        auto type = _description->type();
        Local<Object> description = Nan::New<Object>();
        Nan::Set(description, Nan::New("type").ToLocalChecked(), Nan::New(type).ToLocalChecked());
        Nan::Set(description, Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
        result = description;
      }
    }
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetSignalingState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  auto state = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->signaling_state()
      : SignalingState::kClosed;

  Local<String> value;
  switch (state) {
    case SignalingState::kStable:
      value = Nan::New("stable").ToLocalChecked();
      break;
    case SignalingState::kHaveLocalOffer:
      value = Nan::New("have-local-offer").ToLocalChecked();
      break;
    case SignalingState::kHaveRemoteOffer:
      value = Nan::New("have-remote-offer").ToLocalChecked();
      break;
    case SignalingState::kHaveLocalPrAnswer:
      value = Nan::New("have-local-pranswer").ToLocalChecked();
      break;
    case SignalingState::kHaveRemotePrAnswer:
      value = Nan::New("have-remote-pranswer").ToLocalChecked();
      break;
    case SignalingState::kClosed:
      value = Nan::New("closed").ToLocalChecked();
      break;
  }

  TRACE_END;
  info.GetReturnValue().Set(value);
}

NAN_GETTER(PeerConnection::GetIceConnectionState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  auto state = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : IceConnectionState::kIceConnectionClosed;

  Local<Value> value;
  switch (state) {
    case IceConnectionState::kIceConnectionChecking:
      value = Nan::New("checking").ToLocalChecked();
      break;
    case IceConnectionState::kIceConnectionClosed:
      value = Nan::New("closed").ToLocalChecked();
      break;
    case IceConnectionState::kIceConnectionCompleted:
      value = Nan::New("completed").ToLocalChecked();
      break;
    case IceConnectionState::kIceConnectionConnected:
      value = Nan::New("connected").ToLocalChecked();
      break;
    case IceConnectionState::kIceConnectionDisconnected:
      value = Nan::New("disconnected").ToLocalChecked();
      break;
    case IceConnectionState::kIceConnectionFailed:
      value = Nan::New("failed").ToLocalChecked();
      break;
    case IceConnectionState::kIceConnectionMax:
      TRACE_END;
      return Nan::ThrowTypeError("WebRTC\'s RTCPeerConnection has an ICE connection state \"max\", but I have no idea"
              "what this means. If you see this error, file a bug on https://github.com/js-platform/node-webrtc");
    case IceConnectionState::kIceConnectionNew:
      value = Nan::New("new").ToLocalChecked();
      break;
  }

  TRACE_END;
  info.GetReturnValue().Set(value);
}

NAN_GETTER(PeerConnection::GetIceGatheringState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  auto state = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_gathering_state()
      : IceGatheringState::kIceGatheringComplete;

  Local<Value> value;
  switch (state) {
    case IceGatheringState::kIceGatheringNew:
      value = Nan::New("new").ToLocalChecked();
      break;
    case IceGatheringState::kIceGatheringGathering:
      value = Nan::New("gathering").ToLocalChecked();
      break;
    case IceGatheringState::kIceGatheringComplete:
      value = Nan::New("complete").ToLocalChecked();
      break;
  }

  TRACE_END;
  info.GetReturnValue().Set(value);
}

NAN_SETTER(PeerConnection::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void PeerConnection::Init(Handle<Object> exports) {
  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnection").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  Nan::SetPrototypeMethod(tpl, "createOffer", CreateOffer);
  Nan::SetPrototypeMethod(tpl, "createAnswer", CreateAnswer);
  Nan::SetPrototypeMethod(tpl, "setLocalDescription", SetLocalDescription);
  Nan::SetPrototypeMethod(tpl, "setRemoteDescription", SetRemoteDescription);
  Nan::SetPrototypeMethod(tpl, "getStats", GetStats);
  Nan::SetPrototypeMethod(tpl, "updateIce", UpdateIce);
  Nan::SetPrototypeMethod(tpl, "addIceCandidate", AddIceCandidate);
  Nan::SetPrototypeMethod(tpl, "createDataChannel", CreateDataChannel);
  Nan::SetPrototypeMethod(tpl, "close", Close);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("localDescription").ToLocalChecked(), GetLocalDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remoteDescription").ToLocalChecked(), GetRemoteDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("signalingState").ToLocalChecked(), GetSignalingState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceConnectionState").ToLocalChecked(), GetIceConnectionState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceGatheringState").ToLocalChecked(), GetIceGatheringState, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("PeerConnection").ToLocalChecked(), tpl->GetFunction());
}
