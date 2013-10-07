/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <node_buffer.h>


#undef STATIC_ASSERT  // Node defines this and so do we.

#define USE_FAKE_MEDIA_STREAMS

#include <stdint.h>
#include <iostream>
#include <string>

#include "nspr.h"
#include "nss.h"
#include "ssl.h"

#include "FakeMediaStreams.h"
#include "FakeMediaStreamsImpl.h"
#include "PeerConnectionImpl.h"
#include "PeerConnectionCtx.h"
#include "nsCOMPtr.h"
#include "nsNetCID.h"
#include "nsNetUtil.h"
#include "nsXPCOMGlue.h"
#include "nsXPCOM.h"
#include "nsIIOService.h"
#include "nsWeakReference.h"
#include "nsISocketTransportService.h"
#include "nsPISocketTransportService.h"
#include "nsServiceManagerUtils.h"
#include "TestHarness.h"
#include "runnable_utils.h"

#include "common.h"
#include "peerconnection.h"

using namespace node;
using namespace v8;
using namespace mozilla;

Persistent<Function> PeerConnection::constructor;

// Singleton to hold XPCOM and the PC main thread.
class PeerConnectionSingleton {
 public:
  ~PeerConnectionSingleton() {
    if (sts_)
      sts_->Shutdown();
  }

  static PeerConnectionSingleton* Instance() {
    if (!instance_) {
      instance_ = Create();
    }
    return instance_;
  }

  nsCOMPtr<nsIThread> main_thread() { return main_thread_; }

 private:
  PeerConnectionSingleton() : xpcom_("") {}

  static PeerConnectionSingleton* Create() {
    ScopedDeletePtr<PeerConnectionSingleton> instance(new PeerConnectionSingleton());
    if (!instance->InitServices())
      return nullptr;

    return instance.forget();
  }

  bool InitServices() {
    nsresult rv;
    nsIThread *thread;
    rv = NS_NewNamedThread("pseudo-main",&thread);
    if (NS_FAILED(rv))
      return false;

    main_thread_ = thread;

    NSS_NoDB_Init(NULL);
    NSS_SetDomesticPolicy();

    return true;
  }

  ScopedXPCOM xpcom_;
  nsCOMPtr<nsIIOService> ioservice_;
  nsCOMPtr<nsIEventTarget> sts_target_;
  nsCOMPtr<nsPISocketTransportService> sts_;
  nsCOMPtr<nsIThread> main_thread_;

  static PeerConnectionSingleton* instance_;
};
PeerConnectionSingleton* PeerConnectionSingleton::instance_;

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
    StateEvent(uint32_t state)
    : state(state) {}
    uint32_t state;
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

PeerConnectionObserver::PeerConnectionObserver(PeerConnection* pc)
 : _pc(pc)
{
  /* member initializers and constructor code */
  TRACE_CALL;
  TRACE_END;
}

PeerConnectionObserver::~PeerConnectionObserver()
{
  /* destructor code */
  TRACE_CALL;
  TRACE_END;
}

/* void onCreateOfferSuccess (in string offer); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateOfferSuccess(const char * offer)
{
  TRACE_CALL;
  SDPEvent* data = new SDPEvent(offer);
  _pc->QueueEvent(PeerConnection::CREATE_OFFER_SUCCESS, (void*)data);
  TRACE_END;
  return NS_OK;
}

/* void onCreateOfferError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateOfferError(uint32_t code, const char * message)
{
  TRACE_CALL;
  ErrorEvent* data = new ErrorEvent(code, message);
  _pc->QueueEvent(PeerConnection::CREATE_OFFER_ERROR, (void*)data);
  TRACE_END;
  return NS_OK;
}

/* void onCreateAnswerSuccess (in string answer); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateAnswerSuccess(const char * answer)
{
  TRACE_CALL;
  SDPEvent* data = new SDPEvent(answer);
  _pc->QueueEvent(PeerConnection::CREATE_ANSWER_SUCCESS, (void*)data);
  TRACE_END;
  return NS_OK;
}

/* void onCreateAnswerError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateAnswerError(uint32_t code, const char * message)
{
  TRACE_CALL;
  ErrorEvent* data = new ErrorEvent(code, message);
  _pc->QueueEvent(PeerConnection::CREATE_ANSWER_ERROR, (void*)data);
  TRACE_END;
  return NS_OK;
}

/* void onSetLocalDescriptionSuccess (); */
NS_IMETHODIMP PeerConnectionObserver::OnSetLocalDescriptionSuccess()
{
  TRACE_CALL;
  _pc->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_SUCCESS, (void*)NULL);
  TRACE_END;
  return NS_OK;
}

/* void onSetRemoteDescriptionSuccess (); */
NS_IMETHODIMP PeerConnectionObserver::OnSetRemoteDescriptionSuccess()
{
  TRACE_CALL;
  _pc->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_SUCCESS, (void*)NULL);
  TRACE_END;
  return NS_OK;
}

/* void onSetLocalDescriptionError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnSetLocalDescriptionError(uint32_t code, const char * message)
{
  TRACE_CALL;
  ErrorEvent* data = new ErrorEvent(code, message);
  _pc->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
  return NS_OK;
}

/* void onSetRemoteDescriptionError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnSetRemoteDescriptionError(uint32_t code, const char * message)
{
  TRACE_CALL;
  ErrorEvent* data = new ErrorEvent(code, message);
  _pc->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
  return NS_OK;
}

/* void onAddIceCandidateSuccess (); */
NS_IMETHODIMP PeerConnectionObserver::OnAddIceCandidateSuccess()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onAddIceCandidateError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnAddIceCandidateError(uint32_t code, const char * message)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

/* void notifyDataChannel (in nsIDOMDataChannel channel); */
NS_IMETHODIMP PeerConnectionObserver::NotifyDataChannel(nsIDOMDataChannel *channel)
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

/* void notifyConnection (); */
NS_IMETHODIMP PeerConnectionObserver::NotifyConnection()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

/* void notifyClosedConnection (); */
NS_IMETHODIMP PeerConnectionObserver::NotifyClosedConnection()
{
  return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onStateChange (in unsigned long state); */
NS_IMETHODIMP PeerConnectionObserver::OnStateChange(uint32_t state)
{
  TRACE_CALL;
  StateEvent* data = new StateEvent(state);
  _pc->QueueEvent(PeerConnection::STATE_CHANGE, (void*)data);
  TRACE_END;
  return NS_OK;
}

/* void onAddStream (in nsIDOMMediaStream stream); */
NS_IMETHODIMP PeerConnectionObserver::OnAddStream(nsIDOMMediaStream *stream)
{
  TRACE_CALL;
  TRACE_END;
  return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onRemoveStream (); */
NS_IMETHODIMP PeerConnectionObserver::OnRemoveStream()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onAddTrack (); */
NS_IMETHODIMP PeerConnectionObserver::OnAddTrack()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onRemoveTrack (); */
NS_IMETHODIMP PeerConnectionObserver::OnRemoveTrack()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void foundIceCandidate (in string candidate); */
NS_IMETHODIMP PeerConnectionObserver::OnIceCandidate(uint16_t level, const char * mid, const char * candidate)
{
    TRACE_CALL;
    TRACE_END;
    return NS_ERROR_NOT_IMPLEMENTED;
}

//
// PeerConnection
//

PeerConnection::PeerConnection()
{
  RUN_ON_THREAD(PeerConnectionSingleton::Instance()->main_thread(),
              WrapRunnable(this, &PeerConnection::Init_m),
              NS_DISPATCH_SYNC);
  uv_mutex_init(&lock);
  uv_async_init(uv_default_loop(), &async, Run);
  async.data = this;
}

PeerConnection::~PeerConnection()
{
}

void PeerConnection::Init_m()
{
  _pc = sipcc::PeerConnectionImpl::CreatePeerConnection();
  sipcc::IceConfiguration cfg;

  _pco = new PeerConnectionObserver(this);
  _pc->Initialize(_pco, nullptr, cfg,
                 PeerConnectionSingleton::Instance()->main_thread());
  _fs = new DOMMediaStream();
  _fs->SetHintContents(DOMMediaStream::HINT_CONTENTS_VIDEO | DOMMediaStream::HINT_CONTENTS_AUDIO);
  _pc->AddStream(_fs);
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

  while(!self->_events.empty())
  {
    AsyncEvent evt = self->_events.front();
    self->_events.pop();
    TRACE_U("event type", evt.type);

    if(PeerConnection::ERROR_EVENT & evt.type)
    {
      PeerConnectionObserver::ErrorEvent* data = static_cast<PeerConnectionObserver::ErrorEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onerror")));
      v8::Local<v8::Value> argv[1];
      argv[0] = Exception::Error(String::New(data->message));
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::SDP_EVENT & evt.type)
    {
      PeerConnectionObserver::SDPEvent* data = static_cast<PeerConnectionObserver::SDPEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[1];
      argv[0] = String::New(data->sdp);
      callback->Call(pc, 1, argv);
    } else if(PeerConnection::VOID_EVENT & evt.type)
    {
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsuccess")));
      v8::Local<v8::Value> argv[0];
      callback->Call(pc, 0, argv);
    } else if(PeerConnection::STATE_CHANGE & evt.type)
    {
      PeerConnectionObserver::StateEvent* data = static_cast<PeerConnectionObserver::StateEvent*>(evt.data);
      v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(pc->Get(String::New("onsignalingstatechange")));
      v8::Local<v8::Value> argv[1];
      argv[0] = Number::New(data->state);
      callback->Call(pc, 1, argv);
    }
  }

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

  sipcc::MediaConstraints constraints;
  self->_pc->CreateOffer(constraints);

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::CreateAnswer( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  sipcc::MediaConstraints constraints;
  self->_pc->CreateAnswer(constraints);

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

  PeerConnection::Action action;
  if("offer" == type)
  {
    action = PeerConnection::OFFER;
  } else if("answer" == type)
  {
    action = PeerConnection::ANSWER;
  }

  self->_pc->SetLocalDescription(action, sdp.c_str());

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

  PeerConnection::Action action;
  if("offer" == type)
  {
    action = PeerConnection::OFFER;
  } else if("answer" == type)
  {
    action = PeerConnection::ANSWER;
  }

  self->_pc->SetRemoteDescription(action, sdp.c_str());

  TRACE_END;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::AddIceCandidate( const Arguments& args ) {
  TRACE_CALL;
  HandleScope scope;
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
  self->_pc->GetLocalDescription(&sdp);

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
  self->_pc->GetRemoteDescription(&sdp);

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
  self->_pc->GetSignalingState(&state);

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

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onnegotiationneeded"),
    Null());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onicecandidate"),
    Null());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("onsignalingstatechange"),
    Null());

  // tpl->PrototypeTemplate()->Set(String::NewSymbol("onaddstream"),
  //   Null());

  // tpl->PrototypeTemplate()->Set(String::NewSymbol("onremovestream"),
  //   Null());

  tpl->PrototypeTemplate()->Set(String::NewSymbol("oniceconnectionstatechange"),
    Null());

  tpl->InstanceTemplate()->SetAccessor(String::New("localDescription"), GetLocalDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("remoteDescription"), GetRemoteDescription, ReadOnly);
  tpl->InstanceTemplate()->SetAccessor(String::New("signalingState"), GetSignalingState, ReadOnly);

  Persistent<Function> ctor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("PeerConnection"), ctor );
}
