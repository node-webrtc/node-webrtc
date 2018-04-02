/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peerconnection.h"

#include "webrtc/base/refcountedobject.h"
#include "webrtc/p2p/client/basicportallocator.h"

#include "common.h"
#include "converters/arguments.h"
#include "converters/webrtc.h"
#include "create-answer-observer.h"
#include "create-offer-observer.h"
#include "datachannel.h"
#include "error.h"
#include "functional/maybe.h"
#include "peerconnectionfactory.h"
#include "rtcstatsresponse.h"
#include "set-local-description-observer.h"
#include "set-remote-description-observer.h"
#include "stats-observer.h"

using node_webrtc::ExtendedRTCConfiguration;
using node_webrtc::From;
using node_webrtc::Maybe;
using node_webrtc::PeerConnection;
using node_webrtc::PeerConnectionFactory;
using node_webrtc::RTCSessionDescriptionInit;
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

PeerConnection::PeerConnection(ExtendedRTCConfiguration configuration)
  : loop(uv_default_loop()) {
  _createOfferObserver = new rtc::RefCountedObject<CreateOfferObserver>(this);
  _createAnswerObserver = new rtc::RefCountedObject<CreateAnswerObserver>(this);
  _setLocalDescriptionObserver = new rtc::RefCountedObject<SetLocalDescriptionObserver>(this);
  _setRemoteDescriptionObserver = new rtc::RefCountedObject<SetRemoteDescriptionObserver>(this);

  // TODO(mroberts): Read `factory` (non-standard) from RTCConfiguration?
  _factory = PeerConnectionFactory::GetOrCreateDefault();
  _shouldReleaseFactory = true;

  auto portAllocator = std::unique_ptr<cricket::PortAllocator>(new cricket::BasicPortAllocator(
              _factory->getNetworkManager(),
              _factory->getSocketFactory()));
  _port_range = configuration.portRange;
  portAllocator->SetPortRange(
      _port_range.min.FromMaybe(0),
      _port_range.max.FromMaybe(65535));

  _jinglePeerConnection = _factory->factory()->CreatePeerConnection(
          configuration.configuration,
          std::move(portAllocator),
          nullptr,
          this);

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
  AsyncEvent evt = { type, data };
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
    if (self->_events.empty()) {
      uv_mutex_unlock(&self->lock);
      break;
    }
    auto evt = self->_events.front();
    self->_events.pop();
    uv_mutex_unlock(&self->lock);

    TRACE_U("evt.type", evt.type);
    if (PeerConnection::ERROR_EVENT & evt.type) {
      auto data = static_cast<PeerConnection::ErrorEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onerror").ToLocalChecked()));
      Local<Value> argv[1];
      argv[0] = Nan::Error(data->msg.c_str());
      Nan::MakeCallback(pc, callback, 1, argv);
    } else if (PeerConnection::SDP_EVENT & evt.type) {
      auto data = static_cast<PeerConnection::SdpEvent*>(evt.data);
      self->_lastSdp = data->desc;
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onsuccess").ToLocalChecked()));
      Local<Value> argv[1];
      argv[0] = Nan::New(data->desc.c_str()).ToLocalChecked();
      Nan::MakeCallback(pc, callback, 1, argv);
    } else if (PeerConnection::GET_STATS_SUCCESS & evt.type) {
      auto data = static_cast<PeerConnection::GetStatsEvent*>(evt.data);
      Nan::Callback* callback = data->callback;
      Local<Value> cargv[2];
      cargv[0] = Nan::New<External>(static_cast<void*>(&data->timestamp));
      cargv[1] = Nan::New<External>(static_cast<void*>(&data->reports));
      Local<Value> argv[1];
      argv[0] = Nan::New(RTCStatsResponse::constructor)->NewInstance(2, cargv);
      callback->Call(1, argv);
    } else if (PeerConnection::NEGOTIATION_NEEDED & evt.type) {
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onnegotiationneeded").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Nan::MakeCallback(pc, callback, 0, nullptr);
      }
    } else if (PeerConnection::VOID_EVENT & evt.type) {
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onsuccess").ToLocalChecked()));
      Local<Value> argv[1];
      Nan::MakeCallback(pc, callback, 0, argv);
    } else if (PeerConnection::SIGNALING_STATE_CHANGE & evt.type) {
      auto data = static_cast<PeerConnection::StateEvent*>(evt.data);
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
      auto callback = Local<Function>::Cast(pc->Get(Nan::New("oniceconnectionstatechange").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Nan::MakeCallback(pc, callback, 0, nullptr);
      }
      callback = Local<Function>::Cast(pc->Get(Nan::New("onconnectionstatechange").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Nan::MakeCallback(pc, callback, 0, nullptr);
      }
    } else if (PeerConnection::ICE_GATHERING_STATE_CHANGE & evt.type) {
      auto data = static_cast<PeerConnection::StateEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onicegatheringstatechange").ToLocalChecked()));
      if (!callback.IsEmpty()) {
        Local<Value> argv[1];
        argv[0] = Nan::New<Uint32>(data->state);
        Nan::MakeCallback(pc, callback, 1, argv);
      }
    } else if (PeerConnection::ICE_CANDIDATE & evt.type) {
      auto data = static_cast<PeerConnection::IceEvent*>(evt.data);
      Local<Function> callback = Local<Function>::Cast(pc->Get(Nan::New("onicecandidate").ToLocalChecked()));
      if (!callback.IsEmpty() && data->error.empty()) {
        auto maybeCandidate = From<Local<Value>>(data->candidate.get());
        if (maybeCandidate.IsValid()) {
          Local<Value> argv[1];
          argv[0] = maybeCandidate.UnsafeFromValid();
          Nan::MakeCallback(pc, callback, 1, argv);
        }
      }
    } else if (PeerConnection::NOTIFY_DATA_CHANNEL & evt.type) {
      auto data = static_cast<PeerConnection::DataChannelEvent*>(evt.data);
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
    uv_close(reinterpret_cast<uv_handle_t*>(handle), reinterpret_cast<uv_close_cb>(&PeerConnection::Shutdown));
  }

  TRACE_END;
}

void PeerConnection::Shutdown(uv_async_t* handle) {
  auto self = static_cast<PeerConnection*>(handle->data);
  self->Unref();
}

void PeerConnection::OnError() {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) {
  TRACE_CALL;
  auto data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::SIGNALING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) {
  TRACE_CALL;
  auto data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_CONNECTION_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) {
  TRACE_CALL;
  auto data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_GATHERING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceCandidate(const webrtc::IceCandidateInterface* candidate) {
  TRACE_CALL;
  auto data = new PeerConnection::IceEvent(candidate);
  QueueEvent(PeerConnection::ICE_CANDIDATE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> jingle_data_channel) {
  TRACE_CALL;
  DataChannelObserver* observer = new DataChannelObserver(_factory, jingle_data_channel);
  auto data = new PeerConnection::DataChannelEvent(observer);
  QueueEvent(PeerConnection::NOTIFY_DATA_CHANNEL, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRenegotiationNeeded() {
  TRACE_CALL;
  auto data = new PeerConnection::AsyncEvent();
  QueueEvent(PeerConnection::NEGOTIATION_NEEDED, static_cast<void*>(data));
  TRACE_END;
}

NAN_METHOD(PeerConnection::New) {
  TRACE_CALL;

  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct the PeerConnection.");
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, Maybe<ExtendedRTCConfiguration>);

  // Tell em whats up
  auto obj = new PeerConnection(configuration.FromMaybe(ExtendedRTCConfiguration()));
  obj->Wrap(info.This());
  obj->Ref();

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;

  auto validationOptions = From<Maybe<RTCOfferOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
  [](const Maybe<RTCOfferOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCOfferOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(validationOptions.ToErrors()[0]).ToLocalChecked());
    return;
  }

  auto options = validationOptions.UnsafeFromValid();

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateOffer(self->_createOfferObserver, options.options);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;

  auto validationOptions = From<Maybe<RTCAnswerOptions>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
  [](const Maybe<RTCAnswerOptions> maybeOptions) { return maybeOptions.FromMaybe(RTCAnswerOptions()); });
  if (validationOptions.IsInvalid()) {
    TRACE_END;
    Nan::ThrowTypeError(Nan::New(validationOptions.ToErrors()[0]).ToLocalChecked());
    return;
  }

  auto options = validationOptions.UnsafeFromValid();

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateAnswer(self->_createAnswerObserver, options.options);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;

  CONVERT_ARGS_OR_THROW_AND_RETURN(descriptionInit, RTCSessionDescriptionInit);

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (descriptionInit.sdp.empty()) {
    descriptionInit.sdp = self->_lastSdp;
  }

  CONVERT_OR_THROW_AND_RETURN(descriptionInit, description, SessionDescriptionInterface*);

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

  CONVERT_ARGS_OR_THROW_AND_RETURN(description, SessionDescriptionInterface*);

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

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

  CONVERT_ARGS_OR_THROW_AND_RETURN(candidate, IceCandidateInterface*);

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr && self->_jinglePeerConnection->AddIceCandidate(candidate)) {
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_SUCCESS, static_cast<void*>(nullptr));
  } else {
    delete candidate;
    std::string error = std::string("Failed to set ICE candidate");
    if (self->_jinglePeerConnection == nullptr) {
      error += ", no jingle peer connection";
    }
    error += ".";
    auto data = new PeerConnection::ErrorEvent(error);
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_ERROR, static_cast<void*>(data));
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateDataChannel) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection == nullptr) {
    TRACE_END;
    Nan::ThrowError("Failed to execute 'createDataChannel' on 'RTCPeerConnection': The RTCPeerConnection's signalingState is 'closed'.");
    return;
  }

  CONVERT_ARGS_OR_THROW_AND_RETURN(args, std::tuple<std::string COMMA Maybe<DataChannelInit>>);

  auto label = std::get<0>(args);
  auto dataChannelInit = std::get<1>(args).FromMaybe(DataChannelInit());

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface =
      self->_jinglePeerConnection->CreateDataChannel(label, &dataChannelInit);

  auto observer = new DataChannelObserver(self->_factory, data_channel_interface);

  Local<Value> cargv[1];
  cargv[0] = Nan::New<External>(static_cast<void*>(observer));
  Local<Value> dc = Nan::New(DataChannel::constructor)->NewInstance(1, cargv);

  TRACE_END;
  info.GetReturnValue().Set(dc);
}

NAN_METHOD(PeerConnection::GetConfiguration) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? ExtendedRTCConfiguration(self->_jinglePeerConnection->GetConfiguration(), self->_port_range)
      : self->_cached_configuration,
      configuration,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(configuration);
}

NAN_METHOD(PeerConnection::SetConfiguration) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  CONVERT_ARGS_OR_THROW_AND_RETURN(configuration, RTCConfiguration);

  if (!self->_jinglePeerConnection) {
    TRACE_END;
    Nan::ThrowError("RTCPeerConnection is closed");
    return;
  }

  webrtc::RTCError rtcError;
  if (!self->_jinglePeerConnection->SetConfiguration(configuration, &rtcError)) {
    CONVERT_OR_THROW_AND_RETURN(&rtcError, error, Local<Value>);
    Nan::ThrowError(error);
    return;
  }

  TRACE_END;
}

NAN_METHOD(PeerConnection::GetStats) {
  TRACE_CALL;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

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

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_cached_configuration = ExtendedRTCConfiguration(
            self->_jinglePeerConnection->GetConfiguration(),
            self->_port_range);
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

NAN_GETTER(PeerConnection::GetCanTrickleIceCandidates) {
  TRACE_CALL;
  (void) property;

  TRACE_END;
  info.GetReturnValue().Set(Nan::Null());
}

NAN_GETTER(PeerConnection::GetConnectionState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  auto iceConnectionState = self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : webrtc::PeerConnectionInterface::IceConnectionState::kIceConnectionClosed;

  CONVERT_OR_THROW_AND_RETURN(iceConnectionState, connectionState, RTCPeerConnectionState);
  CONVERT_OR_THROW_AND_RETURN(connectionState, value, Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(value);
}

NAN_GETTER(PeerConnection::GetCurrentLocalDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_local_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetLocalDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->local_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetPendingLocalDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_local_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_local_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetCurrentRemoteDescription) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->current_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->current_remote_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetRemoteDescription) {
  TRACE_CALL;

  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->remote_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetPendingRemoteDescription) {
  TRACE_CALL;

  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  Local<Value> result = Nan::Null();
  if (self->_jinglePeerConnection && self->_jinglePeerConnection->pending_remote_description()) {
    CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection->pending_remote_description(), description, Local<Value>);
    result = description;
  }

  TRACE_END;
  info.GetReturnValue().Set(result);
}

NAN_GETTER(PeerConnection::GetSignalingState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->signaling_state()
      : SignalingState::kClosed,
      state,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
}

NAN_GETTER(PeerConnection::GetIceConnectionState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_connection_state()
      : IceConnectionState::kIceConnectionClosed,
      state,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
}

NAN_GETTER(PeerConnection::GetIceGatheringState) {
  TRACE_CALL;
  (void) property;

  auto self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  CONVERT_OR_THROW_AND_RETURN(self->_jinglePeerConnection
      ? self->_jinglePeerConnection->ice_gathering_state()
      : IceGatheringState::kIceGatheringComplete,
      state,
      Local<Value>);

  TRACE_END;
  info.GetReturnValue().Set(state);
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
  Nan::SetPrototypeMethod(tpl, "getConfiguration", GetConfiguration);
  Nan::SetPrototypeMethod(tpl, "setConfiguration", SetConfiguration);
  Nan::SetPrototypeMethod(tpl, "getStats", GetStats);
  Nan::SetPrototypeMethod(tpl, "updateIce", UpdateIce);
  Nan::SetPrototypeMethod(tpl, "addIceCandidate", AddIceCandidate);
  Nan::SetPrototypeMethod(tpl, "createDataChannel", CreateDataChannel);
  Nan::SetPrototypeMethod(tpl, "close", Close);

  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("canTrickleIceCandidates").ToLocalChecked(), GetCanTrickleIceCandidates, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("connectionState").ToLocalChecked(), GetConnectionState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentLocalDescription").ToLocalChecked(), GetCurrentLocalDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("localDescription").ToLocalChecked(), GetLocalDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingLocalDescription").ToLocalChecked(), GetPendingLocalDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("currentRemoteDescription").ToLocalChecked(), GetCurrentRemoteDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("remoteDescription").ToLocalChecked(), GetRemoteDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("pendingRemoteDescription").ToLocalChecked(), GetPendingRemoteDescription, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("signalingState").ToLocalChecked(), GetSignalingState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceConnectionState").ToLocalChecked(), GetIceConnectionState, ReadOnly);
  Nan::SetAccessor(tpl->InstanceTemplate(), Nan::New("iceGatheringState").ToLocalChecked(), GetIceGatheringState, ReadOnly);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("PeerConnection").ToLocalChecked(), tpl->GetFunction());
}
