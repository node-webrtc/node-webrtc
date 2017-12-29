/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peerconnection.h"

#include "webrtc/api/mediaconstraintsinterface.h"
#include "webrtc/api/test/fakeconstraints.h"
#include "webrtc/base/refcount.h"
#include "webrtc/modules/audio_device/include/fake_audio_device.h"

#include "common.h"
#include "create-answer-observer.h"
#include "create-offer-observer.h"
#include "datachannel.h"
#include "rtcstatsresponse.h"
#include "set-local-description-observer.h"
#include "set-remote-description-observer.h"
#include "stats-observer.h"

using node_webrtc::PeerConnection;
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

Nan::Persistent<Function> PeerConnection::constructor;
rtc::Thread* PeerConnection::_signalingThread;
rtc::Thread* PeerConnection::_workerThread;

//
// PeerConnection
//

PeerConnection::PeerConnection(webrtc::PeerConnectionInterface::IceServers iceServerList)
: loop(uv_default_loop()) {
  _createOfferObserver = new rtc::RefCountedObject<CreateOfferObserver>(this);
  _createAnswerObserver = new rtc::RefCountedObject<CreateAnswerObserver>(this);
  _setLocalDescriptionObserver = new rtc::RefCountedObject<SetLocalDescriptionObserver>(this);
  _setRemoteDescriptionObserver = new rtc::RefCountedObject<SetRemoteDescriptionObserver>(this);

  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  configuration.servers = iceServerList;

  webrtc::FakeConstraints constraints;
  constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, webrtc::MediaConstraintsInterface::kValueTrue);
  // FIXME: crashes without these constraints, why?
  constraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, webrtc::MediaConstraintsInterface::kValueFalse);
  constraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, webrtc::MediaConstraintsInterface::kValueFalse);

  _audioDeviceModule = new webrtc::FakeAudioDeviceModule();
  _jinglePeerConnectionFactory = webrtc::CreatePeerConnectionFactory(_workerThread, _signalingThread, _audioDeviceModule, nullptr, nullptr);
  _jinglePeerConnection = _jinglePeerConnectionFactory->CreatePeerConnection(configuration, &constraints, nullptr, nullptr, this);

  uv_mutex_init(&lock);
  uv_async_init(loop, &async, reinterpret_cast<uv_async_cb>(Run));

  async.data = this;
}

PeerConnection::~PeerConnection() {
  TRACE_CALL;
  _jinglePeerConnection = nullptr;
  _jinglePeerConnectionFactory = nullptr;
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

  PeerConnection* self = static_cast<PeerConnection*>(handle->data);
  TRACE_CALL_P((uintptr_t)self);
  Local<Object> pc = self->handle();
  bool do_shutdown = false;

  while (true) {
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
      Nan::Callback *callback = data->callback;
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
    uv_close(reinterpret_cast<uv_handle_t*>(&self->async), nullptr);
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

void PeerConnection::OnDataChannel(webrtc::DataChannelInterface* jingle_data_channel) {
  TRACE_CALL;
  DataChannelObserver* observer = new DataChannelObserver(jingle_data_channel);
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

  webrtc::PeerConnectionInterface::IceServers iceServerList;

  // Check if we have a configuration object
  if (info[0]->IsObject()) {
    const Local<Object> obj = info[0]->ToObject();

    // Extract keys into array for iteration
    const Local<Array> props = obj->GetPropertyNames();

    // Iterate all of the top-level config keys
    for (uint32_t i = 0; i < props->Length(); i++) {
      // Get the key and value for this particular config field
      const Local<String> key = props->Get(i)->ToString();
      const Local<Value> value = obj->Get(key);

      // Annoyingly convert to std::string
      String::Utf8Value _key(key);
      std::string strKey = std::string(*_key);

      // Handle iceServers configuration
      if (strKey == "iceServers" && value->IsArray()) {
        const Handle<Array> iceServers = Handle<Array>::Cast(value);

        // Iterate over all of the ice servers configured
        for (uint32_t j = 0; j < iceServers->Length(); j++) {
          if (iceServers->Get(j)->IsObject()) {

            const Local<Object> iceServerObj = iceServers->Get(j)->ToObject();
            webrtc::PeerConnectionInterface::IceServer iceServer;

            const Local<Array> iceProps = iceServerObj->GetPropertyNames();

            // Now we have an iceserver object in iceServerObj - Lets iterate all of its fields
            for (uint32_t y = 0; y < iceProps->Length(); y++) {
              String::Utf8Value _iceServerKey(iceProps->Get(y)->ToString());
              std::string iceServerKey = std::string(*_iceServerKey);

              Local<Value> iceValue = iceServerObj->Get(iceProps->Get(y)->ToString());

              // Handle each field by casting the data and assigning to our iceServer intsance
              if ((iceServerKey == "url" || iceServerKey == "urls") && iceValue->IsString()) {
                String::Utf8Value _iceUrl(iceValue->ToString());
                std::string iceUrl = std::string(*_iceUrl);

                iceServer.uri = iceUrl;
              }
              else if ((iceServerKey == "url" || iceServerKey == "urls") && iceValue->IsArray()) {
                Handle<Array> iceUrls = Handle<Array>::Cast(iceValue);

                for (uint32_t x = 0; x < iceUrls->Length(); x++) {
                  String::Utf8Value _iceUrlsEntry(iceUrls->Get(x)->ToString());
                  std::string iceUrlsEntry = std::string(*_iceUrlsEntry);

                  iceServer.urls.push_back(iceUrlsEntry);
                }
              }
              else if (iceServerKey == "credential" && iceValue->IsString()) {
                String::Utf8Value _icePassword(iceValue->ToString());
                std::string icePassword = std::string(*_icePassword);

                iceServer.password = icePassword;
              }
              else if (iceServerKey == "username" && iceValue->IsString()) {
                String::Utf8Value _iceUsername(iceValue->ToString());
                std::string iceUsername = std::string(*_iceUsername);

                iceServer.username = iceUsername;
              }
            }

            // Lastly we push the created ICE server to our iceServerList, to be passed to the 'real' PeerConnection constructor
            iceServerList.push_back(iceServer);
          }
        }
      }
      // else if (strKey == "offerToReceiveAudio") ... Handle more config here. For now i just need ICE
    }
  }

  // Tell em whats up
  PeerConnection* obj = new PeerConnection(iceServerList);
  obj->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateOffer(self->_createOfferObserver, nullptr);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    self->_jinglePeerConnection->CreateAnswer(self->_createAnswerObserver, nullptr);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    Local<Object> desc = Local<Object>::Cast(info[0]);
    String::Utf8Value _type(desc->Get(Nan::New("type").ToLocalChecked())->ToString());
    String::Utf8Value _sdp(desc->Get(Nan::New("sdp").ToLocalChecked())->ToString());

    std::string type = *_type;
    std::string sdp = *_sdp;
    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp, &error);

    self->_jinglePeerConnection->SetLocalDescription(self->_setLocalDescriptionObserver, sdi);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::SetRemoteDescription) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  if (self->_jinglePeerConnection != nullptr) {
    Local<Object> desc = Local<Object>::Cast(info[0]);
    String::Utf8Value _type(desc->Get(Nan::New("type").ToLocalChecked())->ToString());
    String::Utf8Value _sdp(desc->Get(Nan::New("sdp").ToLocalChecked())->ToString());

    std::string type = *_type;
    std::string sdp = *_sdp;
    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp, &error);

    self->_jinglePeerConnection->SetRemoteDescription(self->_setRemoteDescriptionObserver, sdi);
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_METHOD(PeerConnection::AddIceCandidate) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());
  Handle<Object> sdp = Handle<Object>::Cast(info[0]);

  String::Utf8Value _candidate(sdp->Get(Nan::New("candidate").ToLocalChecked())->ToString());
  std::string candidate = *_candidate;
  String::Utf8Value _sipMid(sdp->Get(Nan::New("sdpMid").ToLocalChecked())->ToString());
  std::string sdp_mid = *_sipMid;
  uint32_t sdp_mline_index = sdp->Get(Nan::New("sdpMLineIndex").ToLocalChecked())->Uint32Value();

  webrtc::SdpParseError sdpParseError;
  webrtc::IceCandidateInterface* ci = webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, &sdpParseError);

  if (self->_jinglePeerConnection != nullptr && self->_jinglePeerConnection->AddIceCandidate(ci)) {
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_SUCCESS, static_cast<void*>(nullptr));
  } else {
    std::string error = std::string("Failed to set ICE candidate");
    if(self->_jinglePeerConnection == nullptr) {
      error += ", no jingle peer connection";
    } else if( sdpParseError.description.length() ) {
      error += std::string(", parse error: ") + sdpParseError.description;
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

  String::Utf8Value label(info[0]->ToString());
  Handle<Object> dataChannelDict = Handle<Object>::Cast(info[1]);

  webrtc::DataChannelInit dataChannelInit;
  if (dataChannelDict->Has(Nan::New("id").ToLocalChecked())) {
    Local<Value> value = dataChannelDict->Get(Nan::New("id").ToLocalChecked());
    if (value->IsInt32()) {
      dataChannelInit.id = value->Int32Value();
    }
  }
  if (dataChannelDict->Has(Nan::New("maxRetransmitTime").ToLocalChecked())) {
    Local<Value> value = dataChannelDict->Get(Nan::New("maxRetransmitTime").ToLocalChecked());
    if (value->IsInt32()) {
      dataChannelInit.maxRetransmitTime = value->Int32Value();
    }
  }
  if (dataChannelDict->Has(Nan::New("maxRetransmits").ToLocalChecked())) {
    Local<Value> value = dataChannelDict->Get(Nan::New("maxRetransmits").ToLocalChecked());
    if (value->IsInt32()) {
      dataChannelInit.maxRetransmits = value->Int32Value();
    }
  }
  if (dataChannelDict->Has(Nan::New("negotiated").ToLocalChecked())) {
    Local<Value> value = dataChannelDict->Get(Nan::New("negotiated").ToLocalChecked());
    if (value->IsBoolean()) {
      dataChannelInit.negotiated = value->BooleanValue();
    }
  }
  if (dataChannelDict->Has(Nan::New("ordered").ToLocalChecked())) {
    Local<Value> value = dataChannelDict->Get(Nan::New("ordered").ToLocalChecked());
    if (value->IsBoolean()) {
      dataChannelInit.ordered = value->BooleanValue();
    }
  }
  if (dataChannelDict->Has(Nan::New("protocol").ToLocalChecked())) {
    Local<Value> value = dataChannelDict->Get(Nan::New("protocol").ToLocalChecked());
    if (value->IsString()) {
      dataChannelInit.protocol = *String::Utf8Value(value->ToString());
    }
  }

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface = self->_jinglePeerConnection->CreateDataChannel(*label, &dataChannelInit);
  DataChannelObserver* observer = new DataChannelObserver(data_channel_interface);

  Local<Value> cargv[1];
  cargv[0] = Nan::New<External>(static_cast<void*>(observer));
  Local<Value> dc = Nan::New(DataChannel::constructor)->NewInstance(1, cargv);

  TRACE_END;
  info.GetReturnValue().Set(dc);
}

NAN_METHOD(PeerConnection::GetStats) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.This());

  Nan::Callback *onSuccess = new Nan::Callback(info[0].As<Function>());
  Nan::Callback *onFailure = new Nan::Callback(info[1].As<Function>());
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
  self->_jinglePeerConnectionFactory = nullptr;

  TRACE_END;
  info.GetReturnValue().Set(Nan::Undefined());
}

NAN_GETTER(PeerConnection::GetLocalDescription) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  const webrtc::SessionDescriptionInterface* sdi = nullptr;

  if (self->_jinglePeerConnection != nullptr) {
    sdi = self->_jinglePeerConnection->local_description();
  }

  Handle<Value> value;
  if (nullptr == sdi) {
    value = Nan::Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = Nan::New(sdp.c_str()).ToLocalChecked();
  }

  TRACE_END;
#if NODE_MAJOR_VERSION == 0
  info.GetReturnValue().Set(Nan::New(value));
#else
  info.GetReturnValue().Set(value);
#endif
}

NAN_GETTER(PeerConnection::GetRemoteDescription) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());
  const webrtc::SessionDescriptionInterface* sdi = nullptr;

  if (self->_jinglePeerConnection != nullptr) {
    sdi = self->_jinglePeerConnection->remote_description();
  }

  Handle<Value> value;
  if (nullptr == sdi) {
    value = Nan::Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = Nan::New(sdp.c_str()).ToLocalChecked();
  }

  TRACE_END;
#if NODE_MAJOR_VERSION == 0
  info.GetReturnValue().Set(Nan::New(value));
#else
  info.GetReturnValue().Set(value);
#endif
}

NAN_GETTER(PeerConnection::GetSignalingState) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  webrtc::PeerConnectionInterface::SignalingState state;

  if (self->_jinglePeerConnection != nullptr) {
    state = self->_jinglePeerConnection->signaling_state();
  } else {
    state = webrtc::PeerConnectionInterface::kClosed;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(state));
}

NAN_GETTER(PeerConnection::GetIceConnectionState) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  webrtc::PeerConnectionInterface::IceConnectionState state;

  if (self->_jinglePeerConnection != nullptr) {
    state = self->_jinglePeerConnection->ice_connection_state();
  } else {
    state = webrtc::PeerConnectionInterface::kIceConnectionClosed;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(state));
}

NAN_GETTER(PeerConnection::GetIceGatheringState) {
  TRACE_CALL;

  PeerConnection* self = Nan::ObjectWrap::Unwrap<PeerConnection>(info.Holder());

  webrtc::PeerConnectionInterface::IceGatheringState state;

  if (self->_jinglePeerConnection != nullptr) {
    state = self->_jinglePeerConnection->ice_gathering_state();
  } else {
    state = webrtc::PeerConnectionInterface::kIceGatheringComplete;
  }

  TRACE_END;
  info.GetReturnValue().Set(Nan::New<Number>(static_cast<uint32_t>(state)));
}

NAN_SETTER(PeerConnection::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

void PeerConnection::Init(rtc::Thread* signalingThread, rtc::Thread* workerThread, Handle<Object> exports) {
  _signalingThread = signalingThread;
  _workerThread = workerThread;

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
