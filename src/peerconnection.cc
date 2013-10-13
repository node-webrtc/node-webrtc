/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "peerconnection.h"

using namespace node;
using namespace v8;

Persistent<Function> PeerConnection::constructor;

void CreateOfferObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  SdpEvent* data = new SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateOfferObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  ErrorEvent* data = new ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_ERROR, (void*)data);
  TRACE_END;
}

void CreateAnswerObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  SdpEvent* data = new SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateAnswerObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  ErrorEvent* data = new ErrorEvent(msg);
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
  ErrorEvent* data = new ErrorEvent(msg);
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
  ErrorEvent* data = new ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
}

#if 0
//
// PeerConnectionObserver class
//
class PeerConnectionObserver
: public IPeerConnectionObserver,
  public nsSupportsWeakReference
{
public:
  NS_DECL_THREADSAFE_ISUPPORTS
  NS_DECL_IPEERCONNECTIONOBSERVER

  PeerConnectionObserver(PeerConnection* pc);

  struct ErrorEvent {
    ErrorEvent(const uint32_t code, const char* message)
    : code(code)
    {
      size_t msglen = strlen(message);
      this->message = new char[msglen+1];
      this->message[msglen] = 0;
      strncpy(this->message, message, msglen);
    }
    uint32_t code;
    char* message;
  };

  struct SDPEvent {
    SDPEvent(const char* sdp)
    {
      size_t sdplen = strlen(sdp);
      this->sdp = new char[sdplen+1];
      this->sdp[sdplen] = 0;
      strncpy(this->sdp, sdp, sdplen);
    }
    char* sdp;
  };

  struct StateEvent {
    StateEvent(uint32_t type, uint32_t state)
    : type(type), state(state) {}
    uint32_t type;
    uint32_t state;
  };

  struct ICEEvent {
    ICEEvent(uint32_t level, const char* mid, const char* candidate)
    : level(level)
    {
      size_t len;

      len = strlen(mid);
      this->mid = new char[len+1];
      this->mid[len] = 0;
      strncpy(this->mid, mid, len);

      len = strlen(candidate);
      this->candidate = new char[len+1];
      this->candidate[len] = 0;
      strncpy(this->candidate, candidate, len);
    }
    uint32_t level;
    char* mid;
    char* candidate;
  };

private:
  PeerConnection* _pc;
  virtual ~PeerConnectionObserver();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS2(PeerConnectionObserver, IPeerConnectionObserver,
                                           nsISupportsWeakReference)

NS_IMETHODIMP PeerConnectionObserver::OnStateChange(uint32_t type)
{
  TRACE_CALL;
  uint32_t state;
  StateEvent* data;
  switch(type)
  {
    case IPeerConnectionObserver::kReadyState:
      _pc->_GetReadyState(&state);
      break;
    case IPeerConnectionObserver::kIceState:
      _pc->_GetIceState(&state);
      break;
    case IPeerConnectionObserver::kSdpState:
      // don't care
      break;
    case IPeerConnectionObserver::kSipccState:
      _pc->_GetSipccState(&state);
      break;
    case IPeerConnectionObserver::kSignalingState:
      _pc->_GetSignalingState(&state);
      break;
  }
  data = new StateEvent(type, state);
  _pc->QueueEvent(PeerConnection::STATE_CHANGE, (void*)data);
  TRACE_END;
  return NS_OK;
}
#endif

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

  _peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
      _signalThread, _workerThread, NULL, NULL, NULL );
  _internalPeerConnection = _peerConnectionFactory->CreatePeerConnection(_iceServers, NULL, NULL, this);
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
      ErrorEvent* data = static_cast<ErrorEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onerror")));
      v8::Local<v8::Value> argv[1];
      argv[0] = Exception::Error(String::New(data->msg.c_str()));
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::SDP_EVENT & evt.type)
    {
      SdpEvent* data = static_cast<SdpEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[1];
      argv[0] = String::New(data->desc.c_str());
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::VOID_EVENT & evt.type)
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[0];
      callback->Call(pc, 0, argv);
    } /*else if(PeerConnection::STATE_CHANGE & evt.type)
    {
      PeerConnectionObserver::StateEvent* data = static_cast<PeerConnectionObserver::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onstatechange")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[2];
        argv[0] = Number::New(data->type);
        argv[1] = Number::New(data->state);
        callback->Call(pc, 2, argv);
      }
    }*/ else if(PeerConnection::ICE_CANDIDATE & evt.type)
    {
      IceEvent* data = static_cast<IceEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onicecandidate")));
      if(!callback.IsEmpty())
      {
        v8::Local<v8::Value> argv[3];
        argv[0] = String::New(data->candidate.c_str());
        argv[1] = String::New(data->sdpMid.c_str());
        argv[2] = Integer::New(data->sdpMLineIndex);
        callback->Call(pc, 3, argv);
      }
    }

  }

  TRACE_END;
}

void PeerConnection::OnError() {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState new_state ) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState new_state ) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState new_state ) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnIceStateChange( webrtc::PeerConnectionInterface::IceState new_state ) {
  TRACE_CALL;
  TRACE_END;
}

void PeerConnection::OnStateChange( StateType state_changed ) {
  TRACE_CALL;
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
  IceEvent* data = new IceEvent(candidate);
  QueueEvent(PeerConnection::ICE_CANDIDATE, static_cast<void*>(data));
  TRACE_END;
}

void PeerConnection::OnDataChannel( webrtc::DataChannelInterface* data_channel ) {
  TRACE_CALL;
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
  v8::String::Utf8Value _candidate(args[0]);
  v8::String::Utf8Value _sipMid(args[1]);
  v8::Local<v8::Integer> _sipMLineIndex = v8::Local<v8::Integer>::Cast(args[2]);

  std::string sdp = *_candidate;
  std::string sdp_mid = *_sipMid;
  uint32_t sdp_mline_index = _sipMLineIndex->Value();

  webrtc::IceCandidateInterface* candidate = webrtc::CreateIceCandidate(sdp, sdp_mline_index, sdp_mid);

  if(!self->_internalPeerConnection->AddIceCandidate(candidate))
  {
    INFO("AddIceCandidate failed!\n");
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
  char* sdp = NULL;
  // self->_pc->GetLocalDescription(&sdp);

  Handle<Value> value;
  if(!sdp) {
    value = Null();
  } else {
    value = v8::String::New(sdp);
  }

  TRACE_END;
  return scope.Close(value);
}

Handle<Value> PeerConnection::GetRemoteDescription( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );
  char* sdp = NULL;
  // self->_pc->GetRemoteDescription(&sdp);

  Handle<Value> value;
  if(!sdp) {
    value = Null();
  } else {
    value = v8::String::New(sdp);
  }

  TRACE_END;
  return scope.Close(value);
}

Handle<Value> PeerConnection::GetSignalingState( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  uint32_t state;
  // self->_pc->GetSignalingState(&state);

  TRACE_END;
  return scope.Close(Number::New(state));
}

Handle<Value> PeerConnection::GetReadyState( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  uint32_t state;
  // self->_pc->GetReadyState(&state);

  TRACE_END;
  return scope.Close(Number::New(state));
}

Handle<Value> PeerConnection::GetIceConnectionState( Local<String> property, const AccessorInfo& info ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  uint32_t state;
  // self->_pc->GetIceState(&state);

  TRACE_END;
  return scope.Close(Number::New(state));
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

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onerror"),
    Null());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onsuccess"),
    Null());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onnegotiationneeded"),
    Null());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onicecandidate"),
    Null());

  // tpl->PrototypeTemplate()->Set(String::NewSymbol("onaddstream"),
  //   Null());

  // tpl->PrototypeTemplate()->Set(String::NewSymbol("onremovestream"),
  //   Null());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onstatechange"),
    Null());

  tpl->InstanceTemplate()->SetAccessor(String::New("localDescription"), GetLocalDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("remoteDescription"), GetRemoteDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("readyState"), GetReadyState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("signalingState"), GetSignalingState, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("iceConnectionState"), GetIceConnectionState, ReadOnly);

  Persistent<Function> ctor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("PeerConnection"), ctor );
}
