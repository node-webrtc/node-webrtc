/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"
#include "talk/app/webrtc/test/fakeconstraints.h"
#include "talk/app/webrtc/peerconnectioninterface.h"

#include "common.h"
#include "peerconnection.h"
#include "datachannel.h"
//#include "mediastream.h"
#include "create-offer-observer.h"
#include "create-answer-observer.h"
#include "set-local-description-observer.h"
#include "set-remote-description-observer.h"
#include "stats-observer.h"
#include "rtcstatsresponse.h"

using namespace node;
using namespace v8;
using namespace node_webrtc;

Persistent<Function> PeerConnection::constructor;

//
// PeerConnection
//

PeerConnection::PeerConnection(v8::Handle<v8::Object> configuration, v8::Handle<v8::Object> constraints)
  : loop(uv_default_loop())
{
  _createOfferObserver = new rtc::RefCountedObject<CreateOfferObserver>( this );
  _createAnswerObserver = new rtc::RefCountedObject<CreateAnswerObserver>( this );
  _setLocalDescriptionObserver = new rtc::RefCountedObject<SetLocalDescriptionObserver>( this );
  _setRemoteDescriptionObserver = new rtc::RefCountedObject<SetRemoteDescriptionObserver>( this );

  if (!configuration.IsEmpty()) {
    v8::Local<v8::Value> iceservers_value = configuration->Get(NanNew("iceServers"));
    
    if (!iceservers_value.IsEmpty() && iceservers_value->IsArray()) {
      v8::Local<v8::Array> list = v8::Local<v8::Array>::Cast(iceservers_value);

      for (unsigned int index = 0; index < list->Length(); index++) {
        v8::Local<v8::Value> server_value = list->Get(index);

        if (!server_value.IsEmpty() && server_value->IsObject()) {
          v8::Local<v8::Object> server = v8::Local<v8::Object>::Cast(server_value);

          v8::Local<v8::Value> url_value = server->Get(NanNew("url"));
          v8::Local<v8::Value> username_value = server->Get(NanNew("username"));
          v8::Local<v8::Value> credential_value = server->Get(NanNew("credential"));

          if (!url_value.IsEmpty() && url_value->IsString()) {
            webrtc::PeerConnectionInterface::IceServer entry;
            v8::String::Utf8Value url(url_value->ToString());

            entry.uri = *url;

            if (!username_value.IsEmpty() && username_value->IsString()) {
              v8::String::Utf8Value username(username_value->ToString());
              entry.username = *username;
            }

            if (!credential_value.IsEmpty() && credential_value->IsString()) {
              v8::String::Utf8Value credential(credential_value->ToString());
              entry.password = *credential;
            }

            _iceServers.push_back(entry);
          }
        }        
      }
    }
  }

  bool enableSecure = true;
  bool enableAudio = false;
  bool enableVideo = false;
  
  if (!constraints.IsEmpty()) {
    v8::Local<v8::Value> optional_value = constraints->Get(NanNew("optional"));
    
    if (!optional_value.IsEmpty() && optional_value->IsArray()) {
      v8::Local<v8::Array> options = v8::Local<v8::Array>::Cast(optional_value);
      
      for (unsigned int index = 0; index < options->Length(); index++) {
        v8::Local<v8::Value> option_value = options->Get(index);
        
        if (!option_value.IsEmpty() && option_value->IsObject()) {
          v8::Local<v8::Object> option = v8::Local<v8::Object>::Cast(option_value);
          v8::Local<v8::Value> DtlsSrtpKeyAgreement = option->Get(NanNew("DtlsSrtpKeyAgreement"));
          
          if (!DtlsSrtpKeyAgreement.IsEmpty() && DtlsSrtpKeyAgreement->IsFalse()) {
            enableSecure = false;
          }
        }
      }
    }
/*
    v8::Local<v8::Value> mandatory_value = constraints->Get(NanNew("mandatory"));
    
    if (!mandatory_value.IsEmpty() && mandatory_value->IsObject()) {
      v8::Local<v8::Object> mandatory = v8::Local<v8::Object>::Cast(mandatory_value);
      v8::Local<v8::Value> OfferToReceiveAudio = mandatory->Get(NanNew("OfferToReceiveAudio"));
      v8::Local<v8::Value> OfferToReceiveVideo = mandatory->Get(NanNew("OfferToReceiveVideo"));

      if (!OfferToReceiveAudio.IsEmpty() && OfferToReceiveAudio->IsTrue()) {
        enableAudio = true;
      }
      
      if (!OfferToReceiveVideo.IsEmpty() && OfferToReceiveVideo->IsTrue()) {
        enableVideo = true;
      }
    }
*/
  }
  
  webrtc::FakeConstraints fakeconstraints;
  
  fakeconstraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, enableSecure ? 
                              webrtc::MediaConstraintsInterface::kValueTrue : 
                              webrtc::MediaConstraintsInterface::kValueFalse);
  
  // FIXME: crashes without these constraints, why?
  fakeconstraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveAudio, enableAudio ? 
                               webrtc::MediaConstraintsInterface::kValueTrue : 
                               webrtc::MediaConstraintsInterface::kValueFalse);
                           
  fakeconstraints.AddMandatory(webrtc::MediaConstraintsInterface::kOfferToReceiveVideo, enableVideo ? 
                               webrtc::MediaConstraintsInterface::kValueTrue : 
                               webrtc::MediaConstraintsInterface::kValueFalse);

  _jinglePeerConnectionFactory = webrtc::CreatePeerConnectionFactory();
  _jinglePeerConnection = _jinglePeerConnectionFactory->CreatePeerConnection(_iceServers, &fakeconstraints, NULL, NULL, this);

  uv_mutex_init(&lock);
  uv_async_init(loop, &async, reinterpret_cast<uv_async_cb>(Run));

  async.data = this;
}

PeerConnection::~PeerConnection()
{
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::QueueEvent(AsyncEventType type, void* data)
{
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

void PeerConnection::Run(uv_async_t* handle, int status)
{
  NanScope();

  PeerConnection* self = static_cast<PeerConnection*>(handle->data);
  TRACE_CALL_P((uintptr_t)self);
  v8::Handle<v8::Object> pc = NanObjectWrapHandle(self);
  bool do_shutdown = false;

  while(true)
  {
    uv_mutex_lock(&self->lock);
    bool empty = self->_events.empty();
    
    if(empty)
    {
      uv_mutex_unlock(&self->lock);
      break;
    }
    
    AsyncEvent evt = self->_events.front();
    self->_events.pop();
    uv_mutex_unlock(&self->lock);

    TRACE_U("evt.type", evt.type);
    
    switch (evt.type) {
      case PeerConnection::CREATE_OFFER_SUCCESS:
      case PeerConnection::CREATE_ANSWER_SUCCESS: 
        {
          PeerConnection::SdpEvent* data = static_cast<PeerConnection::SdpEvent*>(evt.data);
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onsuccess")));
          
          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[1];
            argv[0] = NanNew(data->desc);

            NanMakeCallback(pc, callback, 1, argv);
          }
        }
          
        break;
      case PeerConnection::CREATE_OFFER_ERROR:
      case PeerConnection::CREATE_ANSWER_ERROR:
      case PeerConnection::SET_LOCAL_DESCRIPTION_ERROR:
      case PeerConnection::SET_REMOTE_DESCRIPTION_ERROR:
      case PeerConnection::ADD_ICE_CANDIDATE_ERROR:
        {
          PeerConnection::ErrorEvent* data = static_cast<PeerConnection::ErrorEvent*>(evt.data);
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onerror")));
          
          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[1];

            argv[0] = Exception::Error(NanNew(data->msg));

            NanMakeCallback(pc, callback, 1, argv);
          }
        }
        
        break;
      case PeerConnection::SET_LOCAL_DESCRIPTION_SUCCESS:
      case PeerConnection::SET_REMOTE_DESCRIPTION_SUCCESS:
      case PeerConnection::ADD_ICE_CANDIDATE_SUCCESS:
        {
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onsuccess")));
          
          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[0];

            NanMakeCallback(pc, callback, 0, argv);
          }
        }
        
        break;
      case PeerConnection::GET_STATS_SUCCESS:
        {
          PeerConnection::GetStatsEvent* data = static_cast<PeerConnection::GetStatsEvent*>(evt.data);
          NanCallback *callback = data->callback;

          v8::Local<v8::Value> cargv[1];
          v8::Local<v8::Value> argv[1];

          cargv[0] = NanNew<v8::External>(static_cast<void*>(&data->reports));
          argv[0] = NanNew(RTCStatsResponse::constructor)->NewInstance(1, cargv);

          callback->Call(1, argv);
        }
        {
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onsuccess")));
          
          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[0];

            NanMakeCallback(pc, callback, 0, argv);
          }
        }
        
        break;
      case PeerConnection::NOTIFY_DATA_CHANNEL: 
        {
          PeerConnection::DataChannelEvent* data = static_cast<PeerConnection::DataChannelEvent*>(evt.data);
          DataChannelObserver* observer = data->observer;

          v8::Local<v8::Value> cargv[1];
          cargv[0] = NanNew<v8::External>(static_cast<void*>(observer));
          v8::Local<v8::Value> dc = NanNew(DataChannel::constructor)->NewInstance(1, cargv);

          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("ondatachannel")));
          
          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[1];

            argv[0] = dc;

            NanMakeCallback(pc, callback, 1, argv);
          }
        }
        
        break;
      case PeerConnection::NOTIFY_CONNECTION:
        
        break;
      case PeerConnection::NOTIFY_CLOSED_CONNECTION:
        
        break;
      case PeerConnection::ICE_CANDIDATE: 
        {
          PeerConnection::IceEvent* data = static_cast<PeerConnection::IceEvent*>(evt.data);
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onicecandidate")));

          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[3];

            argv[0] = NanNew(data->candidate);
            argv[1] = NanNew(data->sdpMid);
            argv[2] = NanNew<Integer>(data->sdpMLineIndex);

            NanMakeCallback(pc, callback, 3, argv);
          }
        }
        
        break;
      case PeerConnection::SIGNALING_STATE_CHANGE: 
        {
          PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onsignalingstatechange")));

          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[1];

            argv[0] = NanNew<Uint32>(data->state);

            NanMakeCallback(pc, callback, 1, argv);
          }

          if (webrtc::PeerConnectionInterface::kClosed == data->state) {
            do_shutdown = true;
          }
        }
        
        break;
      case PeerConnection::ICE_CONNECTION_STATE_CHANGE: 
        {
          PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("oniceconnectionstatechange")));

          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[1];

            argv[0] = NanNew<Uint32>(data->state);

            NanMakeCallback(pc, callback, 1, argv);
          }
        }
        
        break;
      case PeerConnection::ICE_GATHERING_STATE_CHANGE: 
        {
          PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onicegatheringstatechange")));

          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[1];

            argv[0] = NanNew<Uint32>(data->state);

            NanMakeCallback(pc, callback, 1, argv);
          }
        }
        
        break;
      case PeerConnection::NOTIFY_ADD_STREAM:
/*
        webrtc::MediaStreamInterface* msi = static_cast<webrtc::MediaStreamInterface*>(evt.data);
        v8::Local<v8::Value> cargv[1];
        cargv[0] = NanNew<v8::External>(static_cast<void*>(msi));
        v8::Local<v8::Value> ms = NanNew(MediaStream::constructor)->NewInstance(1, cargv);

        v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onaddstream")));
        
        if(!callback.IsEmpty() && callback->IsFunction()) {
          v8::Local<v8::Value> argv[1];
          argv[0] = ms;
          NanMakeCallback(pc, callback, 1, argv);
        }
*/        
        break;
      case PeerConnection::NOTIFY_REMOVE_STREAM:
/*
        webrtc::MediaStreamInterface* msi = static_cast<webrtc::MediaStreamInterface*>(evt.data);
        v8::Local<v8::Value> cargv[1];
        cargv[0] = NanNew<v8::External>(static_cast<void*>(msi));
        v8::Local<v8::Value> ms = NanNew(MediaStream::constructor)->NewInstance(1, cargv);

        v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onremovestream")));
        
        if(!callback.IsEmpty() && callback->IsFunction()) {
          v8::Local<v8::Value> argv[1];
          argv[0] = ms;
          NanMakeCallback(pc, callback, 1, argv);
        }
*/        
        break;
      case PeerConnection::RENEGOTIATION: 
        {          
          v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(NanNew("onnegotiationneeded")));

          if (!callback.IsEmpty() && callback->IsFunction()) {
            v8::Local<v8::Value> argv[0];
            
            NanMakeCallback(pc, callback, 0, argv);
          }
        }
        
        break;
      default:
        NanThrowTypeError("Unknown WEBRTC Event");
        break;
    }
  }

  if(do_shutdown) {
    uv_close((uv_handle_t*)(&self->async), NULL);
  }

  TRACE_END;
}

void PeerConnection::OnError() {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState new_state ) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::SIGNALING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState new_state ) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_CONNECTION_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState new_state ) {
  TRACE_CALL;
  StateEvent* data = new StateEvent(static_cast<uint32_t>(new_state));
  QueueEvent(PeerConnection::ICE_GATHERING_STATE_CHANGE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnIceCandidate( const webrtc::IceCandidateInterface* candidate ) {
  TRACE_CALL;
  PeerConnection::IceEvent* data = new PeerConnection::IceEvent(candidate);
  QueueEvent(PeerConnection::ICE_CANDIDATE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnDataChannel( webrtc::DataChannelInterface* jingle_data_channel ) {
  TRACE_CALL;
  DataChannelObserver* observer = new DataChannelObserver(jingle_data_channel);
  PeerConnection::DataChannelEvent* data = new PeerConnection::DataChannelEvent(observer);
  QueueEvent(PeerConnection::NOTIFY_DATA_CHANNEL, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnRenegotiationNeeded() {
  TRACE_CALL;
  QueueEvent(PeerConnection::RENEGOTIATION, static_cast<void*>(0));
  TRACE_END;
}

NAN_METHOD(PeerConnection::New) {
  TRACE_CALL;
  NanScope();

  if(!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the PeerConnection.");
  }
  
  v8::Local<v8::Object> configuration;
  v8::Local<v8::Object> constraints;
  
  if (args.Length() >= 1 && args[0]->IsObject()) {
    configuration = v8::Local<v8::Object>::Cast(args[0]);
    
    if (args.Length() >= 2 && args[1]->IsObject()) {
      constraints = v8::Local<v8::Object>::Cast(args[0]);
    }
  }

  PeerConnection* obj = new PeerConnection(configuration, constraints);
  obj->Wrap(args.This());

  TRACE_END;
  NanReturnValue(args.This());
}

NAN_METHOD(PeerConnection::CreateOffer) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  self->_jinglePeerConnection->CreateOffer(self->_createOfferObserver, NULL);

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::CreateAnswer) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  self->_jinglePeerConnection->CreateAnswer(self->_createAnswerObserver, NULL);

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::SetLocalDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::Local<v8::Object> desc = v8::Local<v8::Object>::Cast(args[0]);
  v8::String::Utf8Value _type(desc->Get(NanNew("type"))->ToString());
  v8::String::Utf8Value _sdp(desc->Get(NanNew("sdp"))->ToString());

  std::string type = *_type;
  std::string sdp = *_sdp;
  webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp);

  self->_jinglePeerConnection->SetLocalDescription(self->_setLocalDescriptionObserver, sdi);

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::SetRemoteDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::Local<v8::Object> desc = v8::Local<v8::Object>::Cast(args[0]);
  v8::String::Utf8Value _type(desc->Get(NanNew("type"))->ToString());
  v8::String::Utf8Value _sdp(desc->Get(NanNew("sdp"))->ToString());

  std::string type = *_type;
  std::string sdp = *_sdp;
  webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp);

  self->_jinglePeerConnection->SetRemoteDescription(self->_setRemoteDescriptionObserver, sdi);

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::AddIceCandidate) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  Handle<Object> sdp = Handle<Object>::Cast(args[0]);

  v8::String::Utf8Value _candidate(sdp->Get(NanNew("candidate"))->ToString());
  std::string candidate = *_candidate;
  v8::String::Utf8Value _sipMid(sdp->Get(NanNew("sdpMid"))->ToString());
  std::string sdp_mid = *_sipMid;
  uint32_t sdp_mline_index = sdp->Get(NanNew("sdpMLineIndex"))->Uint32Value();

  webrtc::SdpParseError sdpParseError;
  webrtc::IceCandidateInterface* ci = webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, &sdpParseError);

  if(self->_jinglePeerConnection->AddIceCandidate(ci))
  {
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_SUCCESS, static_cast<void*>(NULL));
  } else
  {
    PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(std::string("Failed to set ICE candidate."));
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_ERROR, static_cast<void*>(data));
  }

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::CreateDataChannel) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::String::Utf8Value label(args[0]->ToString());
  Handle<Object> dataChannelDict = Handle<Object>::Cast(args[1]);

  webrtc::DataChannelInit dataChannelInit;
  if(dataChannelDict->Has(NanNew("id"))) {
    Local<Value> value = dataChannelDict->Get(NanNew("id"));
    if(value->IsInt32()) {
      dataChannelInit.id = value->Int32Value();
    }
  }
  if(dataChannelDict->Has(NanNew("maxRetransmitTime"))) {
    Local<Value> value = dataChannelDict->Get(NanNew("maxRetransmitTime"));
    if(value->IsInt32()) {
      dataChannelInit.maxRetransmitTime = value->Int32Value();
    }
  }
  if(dataChannelDict->Has(NanNew("maxRetransmits"))) {
    Local<Value> value = dataChannelDict->Get(NanNew("maxRetransmits"));
    if(value->IsInt32()) {
      dataChannelInit.maxRetransmits = value->Int32Value();
    }
  }
  if(dataChannelDict->Has(NanNew("negotiated"))) {
    Local<Value> value = dataChannelDict->Get(NanNew("negotiated"));
    if(value->IsBoolean()) {
      dataChannelInit.negotiated = value->BooleanValue();
    }
  }
  if(dataChannelDict->Has(NanNew("ordered"))) {
    Local<Value> value = dataChannelDict->Get(NanNew("ordered"));
    if(value->IsBoolean()) {
      dataChannelInit.ordered = value->BooleanValue();
    }
  }
  if(dataChannelDict->Has(NanNew("protocol"))) {
    Local<Value> value = dataChannelDict->Get(NanNew("protocol"));
    if(value->IsString()) {
      dataChannelInit.protocol = *String::Utf8Value(value->ToString());
    }
  }

  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_interface = self->_jinglePeerConnection->CreateDataChannel(*label, &dataChannelInit);
  DataChannelObserver* observer = new DataChannelObserver(data_channel_interface);

  v8::Local<v8::Value> cargv[1];
  cargv[0] = NanNew<v8::External>(static_cast<void*>(observer));
  v8::Local<v8::Value> dc = NanNew(DataChannel::constructor)->NewInstance(1, cargv);

  TRACE_END;
  NanReturnValue(dc);
}

/*
NAN_METHOD(PeerConnection::AddStream) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  MediaStream* ms = ObjectWrap::Unwrap<MediaStream>( args[0]->ToObject() );
  Handle<Object> constraintsDict = Handle<Object>::Cast(args[1]);

  self->_jinglePeerConnection->AddStream(ms->GetInterface(),NULL);

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::RemoveStream) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  MediaStream* ms = ObjectWrap::Unwrap<MediaStream>( args[0]->ToObject() );

  self->_jinglePeerConnection->RemoveStream(ms->GetInterface());

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::GetLocalStreams) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  rtc::scoped_refptr<webrtc::StreamCollectionInterface> _streams = self->_jinglePeerConnection->local_streams();

  v8::Local<v8::Array> array = NanNew<v8::Array>(_streams->count());
  for (unsigned int index = 0; index < _streams->count(); index++) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = NanNew<v8::External>(static_cast<void*>(_streams->at(index)));
    array->Set(index, NanNew(MediaStream::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  NanReturnValue(array);
}

NAN_METHOD(PeerConnection::GetRemoteStreams) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  rtc::scoped_refptr<webrtc::StreamCollectionInterface> _streams = self->_jinglePeerConnection->remote_streams();

  v8::Local<v8::Array> array = NanNew<v8::Array>(_streams->count());
  for (unsigned int index = 0; index < _streams->count(); index++) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = NanNew<v8::External>(static_cast<void*>(_streams->at(index)));
    array->Set(index, NanNew(MediaStream::constructor)->NewInstance(1, cargv));
  }

  TRACE_END;
  NanReturnValue(array);
}

NAN_METHOD(PeerConnection::GetStreamById) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::String::Utf8Value param1(args[0]->ToString());
  std::string _id = std::string(*param1);
  rtc::scoped_refptr<webrtc::StreamCollectionInterface> _local = self->_jinglePeerConnection->local_streams();
  rtc::scoped_refptr<webrtc::StreamCollectionInterface> _remote = self->_jinglePeerConnection->remote_streams();
  webrtc::MediaStreamInterface* stream = _local->find(_id);
  if (!stream) {
      stream = _remote->find(_id);
  }

  TRACE_END;
  if (stream) {
    v8::Local<v8::Value> cargv[1];
    cargv[0] = NanNew<v8::External>(static_cast<void*>(stream));
    v8::Local<v8::Value> ms = NanNew(MediaStream::constructor)->NewInstance(1, cargv);
    NanReturnValue(ms);
  } else {
    NanReturnValue(NanUndefined());
  }
}
*/
NAN_METHOD(PeerConnection::GetStats) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  NanCallback *onSuccess = new NanCallback(args[0].As<Function>());
  NanCallback *onFailure = new NanCallback(args[1].As<Function>());
  rtc::scoped_refptr<StatsObserver> statsObserver =
     new rtc::RefCountedObject<StatsObserver>( self, onSuccess );

  if (!self->_jinglePeerConnection->GetStats(statsObserver,
    webrtc::PeerConnectionInterface::kStatsOutputLevelStandard))
  {
    // TODO: Include error?
    Local<Value> argv[] = {
      NanNull()
    };
    onFailure->Call(1, argv);
  }

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::UpdateIce) {
  TRACE_CALL;
  NanScope();
  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_METHOD(PeerConnection::Close) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  self->_jinglePeerConnection->Close();

  TRACE_END;
  NanReturnValue(NanUndefined());
}

NAN_GETTER(PeerConnection::GetLocalDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );
  const webrtc::SessionDescriptionInterface* sdi = self->_jinglePeerConnection->local_description();

  Handle<Value> value;
  if(NULL == sdi) {
    value = NanNull();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = NanNew(sdp);
  }

  TRACE_END;
  NanReturnValue(value);
}

NAN_GETTER(PeerConnection::GetRemoteDescription) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );
  const webrtc::SessionDescriptionInterface* sdi = self->_jinglePeerConnection->remote_description();

  Handle<Value> value;
  if(NULL == sdi) {
    value = NanNull();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = NanNew(sdp);
  }

  TRACE_END;
  NanReturnValue(value);
}

NAN_GETTER(PeerConnection::GetSignalingState) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );

  webrtc::PeerConnectionInterface::SignalingState state = self->_jinglePeerConnection->signaling_state();

  TRACE_END;
  NanReturnValue(NanNew<Number>(state));
}

NAN_GETTER(PeerConnection::GetIceConnectionState) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );

  webrtc::PeerConnectionInterface::IceConnectionState state = self->_jinglePeerConnection->ice_connection_state();

  TRACE_END;
  NanReturnValue(NanNew<Number>(state));
}

NAN_GETTER(PeerConnection::GetIceGatheringState) {
  TRACE_CALL;
  NanScope();

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.Holder() );

  webrtc::PeerConnectionInterface::IceGatheringState state = self->_jinglePeerConnection->ice_gathering_state();

  TRACE_END;
  NanReturnValue(NanNew<Number>(static_cast<uint32_t>(state)));
}

NAN_SETTER(PeerConnection::ReadOnly) {
  INFO("PeerConnection::ReadOnly");
}

#define USE_BACKTRACE 1

#ifdef USE_BACKTRACE
#ifdef __APPLE__
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <cxxabi.h>

static void ShowBacktrace(const char *event) {
  void *stack[50] = {0};
  int count = 0, cur = 0, skip = 2;
  
  count = backtrace(stack, sizeof(stack));
  
  if (count) {
    for (int index = (count - 1); index > skip && index >= 1; index--) {
      char addr[128] = {0};
      Dl_info info;
     
      snprintf(addr, sizeof(addr), "%p", stack[index]);
     
      if (dladdr(stack[index], &info) && info.dli_sname) {
        char buffer[128] = {0};
        size_t len = sizeof(buffer);
        int status = 0;

        (void) abi::__cxa_demangle(info.dli_sname, buffer, &len, &status);
        
        if (!status) {
          printf("%d: %s [%s] %s\n", cur, event, addr, buffer);
        } else {
          printf("%d: %s [%s] %s\n", cur, event, addr, info.dli_sname);
        }
      } else {
        printf("%d: %s [%s] function()\n", cur, event, addr);
      }
      
      cur++;
    }
  }
}

struct sigaction actsegv, actbus, actabrt;

void CloseBacktrace() {
  actsegv.sa_handler = SIG_DFL;
  actbus.sa_handler = SIG_DFL;
  actabrt.sa_handler = SIG_DFL;
  
  sigaction(SIGSEGV, &actsegv, 0);
  sigaction(SIGBUS, &actbus, 0);
  sigaction(SIGABRT, &actabrt, 0);
}

static void onSegv(int sig) {
  ShowBacktrace("SIGSEGV");
  CloseBacktrace();
}

static void onBus(int sig) {
  ShowBacktrace("SIGBUS");
  CloseBacktrace();
}

static void onAbort(int sig) {
  ShowBacktrace("SIGABRT");
  CloseBacktrace();
}

void InitBacktrace() {
  sigemptyset(&actsegv.sa_mask);
  sigemptyset(&actbus.sa_mask);
  sigemptyset(&actabrt.sa_mask);
  
  actsegv.sa_flags = 0;
  actsegv.sa_handler = onSegv;
  
  actbus.sa_flags = 0;
  actbus.sa_handler = onBus;
  
  actabrt.sa_flags = 0;
  actabrt.sa_handler = onAbort; 

  sigaction(SIGSEGV, &actsegv, 0);
  sigaction(SIGBUS, &actbus, 0);
  sigaction(SIGABRT, &actabrt, 0);
}

#else

void InitBacktrace() {

}

#endif
#endif

void PeerConnection::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = NanNew<FunctionTemplate>( New );
  tpl->SetClassName( NanNew( "PeerConnection" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( NanNew( "createOffer" ),
    NanNew<FunctionTemplate>( CreateOffer )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "createAnswer" ),
    NanNew<FunctionTemplate>( CreateAnswer )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "setLocalDescription" ),
    NanNew<FunctionTemplate>( SetLocalDescription )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "setRemoteDescription" ),
    NanNew<FunctionTemplate>( SetRemoteDescription )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "getStats" ),
    NanNew<FunctionTemplate>( GetStats )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "updateIce" ),
    NanNew<FunctionTemplate>( UpdateIce )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "addIceCandidate" ),
    NanNew<FunctionTemplate>( AddIceCandidate )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "createDataChannel" ),
    NanNew<FunctionTemplate>( CreateDataChannel )->GetFunction() );

  tpl->PrototypeTemplate()->Set( NanNew( "close" ),
    NanNew<FunctionTemplate>( Close )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(NanNew("localDescription"), GetLocalDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(NanNew("remoteDescription"), GetRemoteDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(NanNew("signalingState"), GetSignalingState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(NanNew("iceConnectionState"), GetIceConnectionState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(NanNew("iceGatheringState"), GetIceGatheringState, ReadOnly);

  NanAssignPersistent(constructor,  tpl->GetFunction() );
  exports->Set( NanNew("PeerConnection"), tpl->GetFunction() );
  
#ifdef USE_BACKTRACE
  InitBacktrace();
#endif
}
