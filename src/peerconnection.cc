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

#include "common.h"
#include "peerconnection.h"
#include "datachannel.h"

using namespace node;
using namespace v8;

Persistent<Function> PeerConnection::constructor;

void CreateOfferObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  PeerConnection::SdpEvent* data = new PeerConnection::SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateOfferObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_ERROR, (void*)data);
  TRACE_END;
}

void CreateAnswerObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  PeerConnection::SdpEvent* data = new PeerConnection::SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateAnswerObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_ERROR, (void*)data);
  TRACE_END;
}

void SetLocalDescriptionObserver::OnSuccess()
{
  TRACE_CALL;
  parent->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_SUCCESS, static_cast<void*>(NULL));
  TRACE_END;
}

void SetLocalDescriptionObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
}

void SetRemoteDescriptionObserver::OnSuccess()
{
  TRACE_CALL;
  parent->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_SUCCESS, static_cast<void*>(NULL));
  TRACE_END;
}

void SetRemoteDescriptionObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
}

//
// PeerConnection
//

PeerConnection::PeerConnection()
{
  uv_mutex_init(&lock);
  uv_async_init(uv_default_loop(), &async, Run);

  async.data = this;

  _createOfferObserver = new talk_base::RefCountedObject<CreateOfferObserver>( this );
  _createAnswerObserver = new talk_base::RefCountedObject<CreateAnswerObserver>( this );
  _setLocalDescriptionObserver = new talk_base::RefCountedObject<SetLocalDescriptionObserver>( this );
  _setRemoteDescriptionObserver = new talk_base::RefCountedObject<SetRemoteDescriptionObserver>( this );

  _signalThread = new talk_base::Thread;
  _workerThread = new talk_base::Thread;

  _signalThread->Start();
  _workerThread->Start();

  webrtc::PeerConnectionInterface::IceServer iceServer;
  iceServer.uri = "stun:stun.l.google.com:19302";
  _iceServers.push_back(iceServer);

  webrtc::FakeConstraints constraints;
  constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableDtlsSrtp, webrtc::MediaConstraintsInterface::kValueTrue);
  // constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableSctpDataChannels, true);
  constraints.AddOptional(webrtc::MediaConstraintsInterface::kEnableRtpDataChannels, webrtc::MediaConstraintsInterface::kValueTrue);

  _peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
      _signalThread, _workerThread, NULL, NULL, NULL );
  _internalPeerConnection = _peerConnectionFactory->CreatePeerConnection(_iceServers, &constraints, NULL, this);
}

PeerConnection::~PeerConnection()
{
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
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = static_cast<PeerConnection*>(handle->data);
  v8::Persistent<v8::Object> pc = self->handle_;

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
    if(PeerConnection::ERROR_EVENT & evt.type)
    {
      PeerConnection::ErrorEvent* data = static_cast<PeerConnection::ErrorEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onerror")));
      v8::Local<v8::Value> argv[1];
      argv[0] = Exception::Error(String::New(data->msg.c_str()));
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::SDP_EVENT & evt.type)
    {
      PeerConnection::SdpEvent* data = static_cast<PeerConnection::SdpEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[1];
      argv[0] = String::New(data->desc.c_str());
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::VOID_EVENT & evt.type)
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[0];
      callback->Call(pc, 0, argv);
    } else if(PeerConnection::SIGNALING_STATE_CHANGE & evt.type)
    {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsignalingstatechange")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = Uint32::New(data->state);
        callback->Call(pc, 1, argv);
      }
    } else if(PeerConnection::ICE_CONNECTION_STATE_CHANGE & evt.type)
    {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("oniceconnectionstatechange")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = Uint32::New(data->state);
        callback->Call(pc, 1, argv);
      }
    } else if(PeerConnection::ICE_GATHERING_STATE_CHANGE & evt.type)
    {
      PeerConnection::StateEvent* data = static_cast<PeerConnection::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onicegatheringstatechange")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = Uint32::New(data->state);
        callback->Call(pc, 1, argv);
      }
    } else if(PeerConnection::ICE_CANDIDATE & evt.type)
    {
      PeerConnection::IceEvent* data = static_cast<PeerConnection::IceEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onicecandidate")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[3];
        argv[0] = String::New(data->candidate.c_str());
        argv[1] = String::New(data->sdpMid.c_str());
        argv[2] = Integer::New(data->sdpMLineIndex);
        callback->Call(pc, 3, argv);
      }
    } else if(PeerConnection::NOTIFY_DATA_CHANNEL & evt.type)
    {
      webrtc::DataChannelInterface* dci = static_cast<webrtc::DataChannelInterface*>(evt.data);
      v8::Local<v8::Value> cargv[1];
      cargv[0] = v8::External::New(static_cast<void*>(dci));
      v8::Local<v8::Value> dc = DataChannel::constructor->NewInstance(1, cargv);

      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("ondatachannel")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[1];
        argv[0] = dc;
        callback->Call(pc, 1, argv);
      }
    }
    // FIXME: delete event
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

void PeerConnection::OnAddStream( webrtc::MediaStreamInterface* stream ) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnRemoveStream( webrtc::MediaStreamInterface* stream ) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnIceCandidate( const webrtc::IceCandidateInterface* candidate ) {
  TRACE_CALL;
  PeerConnection::IceEvent* data = new PeerConnection::IceEvent(candidate);
  QueueEvent(PeerConnection::ICE_CANDIDATE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnDataChannel( webrtc::DataChannelInterface* data_channel ) {
  TRACE_CALL;
  data_channel->AddRef();
  QueueEvent(PeerConnection::NOTIFY_DATA_CHANNEL, static_cast<void*>(data_channel));
  TRACE_END;
}

Handle<Value> PeerConnection::New( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  if(!args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
          String::New("Use the new operator to construct the PeerConnection.")));
  }

  PeerConnection* obj = new PeerConnection();
  obj->Wrap( args.This() );

  TRACE_END;
  return scope.Close( args.This() );
}

Handle<Value> PeerConnection::CreateOffer( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  self->_internalPeerConnection->CreateOffer(self->_createOfferObserver, NULL);

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::CreateAnswer( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  self->_internalPeerConnection->CreateAnswer(self->_createAnswerObserver, NULL);

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::SetLocalDescription( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::Local<v8::Object> desc = v8::Local<v8::Object>::Cast(args[0]);
  v8::String::Utf8Value _type(desc->Get(v8::String::NewSymbol("type"))->ToString());
  v8::String::Utf8Value _sdp(desc->Get(v8::String::NewSymbol("sdp"))->ToString());

  std::string type = *_type;
  std::string sdp = *_sdp;
  webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp);

  self->_internalPeerConnection->SetLocalDescription(self->_setLocalDescriptionObserver, sdi);

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::SetRemoteDescription( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  v8::Local<v8::Object> desc = v8::Local<v8::Object>::Cast(args[0]);
  v8::String::Utf8Value _type(desc->Get(v8::String::NewSymbol("type"))->ToString());
  v8::String::Utf8Value _sdp(desc->Get(v8::String::NewSymbol("sdp"))->ToString());

  std::string type = *_type;
  std::string sdp = *_sdp;
  webrtc::SessionDescriptionInterface* sdi = webrtc::CreateSessionDescription(type, sdp);

  self->_internalPeerConnection->SetRemoteDescription(self->_setRemoteDescriptionObserver, sdi);

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::AddIceCandidate( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  Handle<Object> sdp = Handle<Object>::Cast(args[0]);

  v8::String::Utf8Value _candidate(sdp->Get(String::New("candidate"))->ToString());
  std::string candidate = *_candidate;
  v8::String::Utf8Value _sipMid(sdp->Get(String::New("sdpMid"))->ToString());
  std::string sdp_mid = *_sipMid;
  uint32_t sdp_mline_index = sdp->Get(String::New("sdpMLineIndex"))->Uint32Value();

  webrtc::SdpParseError sdpParseError;
  webrtc::IceCandidateInterface* ci = webrtc::CreateIceCandidate(sdp_mid, sdp_mline_index, candidate, &sdpParseError);

  if(self->_internalPeerConnection->AddIceCandidate(ci))
  {
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_SUCCESS, static_cast<void*>(NULL));
  } else
  {
    PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(std::string("Failed to set ICE candidate."));
    self->QueueEvent(PeerConnection::ADD_ICE_CANDIDATE_ERROR, static_cast<void*>(data));
  }

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::UpdateIce( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;
  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::Close( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;
  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::GetLocalDescription( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );
  const webrtc::SessionDescriptionInterface* sdi = self->_internalPeerConnection->local_description();

  Handle<Value> value;
  if(NULL == sdi) {
    value = Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = v8::String::New(sdp.c_str());
  }

  TRACE_END;
  return scope.Close(value);
}

Handle<Value> PeerConnection::GetRemoteDescription( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );
  const webrtc::SessionDescriptionInterface* sdi = self->_internalPeerConnection->remote_description();

  Handle<Value> value;
  if(NULL == sdi) {
    value = Null();
  } else {
    std::string sdp;
    sdi->ToString(&sdp);
    value = v8::String::New(sdp.c_str());
  }

  TRACE_END;
  return scope.Close(value);
}

Handle<Value> PeerConnection::GetSignalingState( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  webrtc::PeerConnectionInterface::SignalingState state = self->_internalPeerConnection->signaling_state();

  TRACE_END;
  return scope.Close(Number::New(state));
}

Handle<Value> PeerConnection::GetIceConnectionState( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  webrtc::PeerConnectionInterface::IceConnectionState state = self->_internalPeerConnection->ice_connection_state();

  TRACE_END;
  return scope.Close(Number::New(state));
}

Handle<Value> PeerConnection::GetIceGatheringState( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  webrtc::PeerConnectionInterface::IceGatheringState state = self->_internalPeerConnection->ice_gathering_state();

  TRACE_END;
  return scope.Close(Number::New(static_cast<uint32_t>(state)));
}

void PeerConnection::ReadOnly( Local<String> property, Local<Value> value, const AccessorInfo& info ) {
  INFO("PeerConnection::ReadOnly");
}

void PeerConnection::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "PeerConnection" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createOffer" ),
    FunctionTemplate::New( CreateOffer )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createAnswer" ),
    FunctionTemplate::New( CreateAnswer )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setLocalDescription" ),
    FunctionTemplate::New( SetLocalDescription )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setRemoteDescription" ),
    FunctionTemplate::New( SetRemoteDescription )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "updateIce" ),
    FunctionTemplate::New( UpdateIce )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "addIceCandidate" ),
    FunctionTemplate::New( AddIceCandidate )->GetFunction() );

  // tpl->PrototypeTemplate()->Set( String::NewSymbol( "getLocalStreams" ),
  //     FunctionTemplate::New( GetLocalStreams )->GetFunction() );

  // tpl->PrototypeTemplate()->Set( String::NewSymbol( "getRemoteStreams" ),
  //     FunctionTemplate::New( GetRemtoeStreams )->GetFunction() );

  // tpl->PrototypeTemplate()->Set( String::NewSymbol( "getStreamById" ),
  //     FunctionTemplate::New( GetStreamById )->GetFunction() );

  // tpl->PrototypeTemplate()->Set( String::NewSymbol( "addStream" ),
  //     FunctionTemplate::New( AddStream )->GetFunction() );

  // tpl->PrototypeTemplate()->Set( String::NewSymbol( "removeStream" ),
  //     FunctionTemplate::New( RemoveStream )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "close" ),
    FunctionTemplate::New( Close )->GetFunction() );

  tpl->InstanceTemplate()->SetAccessor(String::New("localDescription"), GetLocalDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("remoteDescription"), GetRemoteDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("signalingState"), GetSignalingState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("iceConnectionState"), GetIceConnectionState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("iceGatheringState"), GetIceGatheringState, ReadOnly);

  constructor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("PeerConnection"), constructor );
}
