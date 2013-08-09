/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <node_buffer.h>


#undef STATIC_ASSERT  // Node defines this and so do we.

#define USE_FAKE_MEDIA_STREAMS

#include <stdint.h>
#include <iostream>

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

#if 0
Local<String> ToV8String( webrtc::PeerConnectionInterface::SignalingState state ) {
  switch( state ) {
    case webrtc::PeerConnectionInterface::kStable:
      return String::New( "stable" );
    case webrtc::PeerConnectionInterface::kHaveLocalOffer:
      return String::New( "have-local-offer" );
    case webrtc::PeerConnectionInterface::kHaveRemoteOffer:
      return String::New( "have-remote-offer" );
    case webrtc::PeerConnectionInterface::kHaveLocalPrAnswer:
      return String::New( "have-local-pranswer" );
    case webrtc::PeerConnectionInterface::kHaveRemotePrAnswer:
      return String::New( "have-remote-pranswer" );
    case webrtc::PeerConnectionInterface::kClosed:
      return String::New( "closed" );
    default:
      return String::New( "unknown" );
  }
};

Local<String> ToV8String( webrtc::PeerConnectionInterface::IceGatheringState state ) {
  switch( state ) {
    case webrtc::PeerConnectionInterface::kIceGatheringNew:
      return String::New( "new" );
    case webrtc::PeerConnectionInterface::kIceGatheringGathering:
      return String::New( "gathering" );
    case webrtc::PeerConnectionInterface::kIceGatheringComplete:
      return String::New( "complete" );
    default:
      return String::New( "unknown" );
  }
};

Local<String> ToV8String( webrtc::PeerConnectionInterface::IceConnectionState state ) {
  switch( state ) {
    case webrtc::PeerConnectionInterface::kIceConnectionNew:
      return String::New( "new" );
    case webrtc::PeerConnectionInterface::kIceConnectionChecking:
      return String::New( "checking" );
    case webrtc::PeerConnectionInterface::kIceConnectionConnected:
      return String::New( "connected" );
    case webrtc::PeerConnectionInterface::kIceConnectionCompleted:
      return String::New( "completed" );
    case webrtc::PeerConnectionInterface::kIceConnectionFailed:
      return String::New( "failed" );
    case webrtc::PeerConnectionInterface::kIceConnectionDisconnected:
      return String::New( "disconnected" );
    case webrtc::PeerConnectionInterface::kIceConnectionClosed:
      return String::New( "closed" );
    default:
      return String::New( "unknown" );
  }
};
#endif

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
    ScopedDeletePtr<PeerConnectionSingleton> instance(
        new PeerConnectionSingleton());
    if (!instance->InitServices())
      return nullptr;
    
    return instance.forget();
  }

  bool InitServices() {
    nsresult rv;

#if 0
    ioservice_ = do_GetIOService(&rv);
~Pee    NS_ENSURE_SUCCESS(rv, false);
    sts_target_ = do_GetService(NS_SOCKETTRANSPORTSERVICE_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, false);
    sts_ = do_GetService(NS_SOCKETTRANSPORTSERVICE_CONTRACTID, &rv);
    NS_ENSURE_SUCCESS(rv, false);
#endif

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
class PeerConnectionObserver : public IPeerConnectionObserver
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_IPEERCONNECTIONOBSERVER

  PeerConnectionObserver();

private:
  virtual ~PeerConnectionObserver();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(PeerConnectionObserver, IPeerConnectionObserver)

PeerConnectionObserver::PeerConnectionObserver()
{
  /* member initializers and constructor code */
}

PeerConnectionObserver::~PeerConnectionObserver()
{
  /* destructor code */
}

/* void onCreateOfferSuccess (in string offer); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateOfferSuccess(const char * offer)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onCreateOfferError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateOfferError(uint32_t name, const char * message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onCreateAnswerSuccess (in string answer); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateAnswerSuccess(const char * answer)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onCreateAnswerError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnCreateAnswerError(uint32_t name, const char * message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onSetLocalDescriptionSuccess (); */
NS_IMETHODIMP PeerConnectionObserver::OnSetLocalDescriptionSuccess()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onSetRemoteDescriptionSuccess (); */
NS_IMETHODIMP PeerConnectionObserver::OnSetRemoteDescriptionSuccess()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onSetLocalDescriptionError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnSetLocalDescriptionError(uint32_t name, const char * message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onSetRemoteDescriptionError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnSetRemoteDescriptionError(uint32_t name, const char * message)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onAddIceCandidateSuccess (); */
NS_IMETHODIMP PeerConnectionObserver::OnAddIceCandidateSuccess()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onAddIceCandidateError (in unsigned long name, in string message); */
NS_IMETHODIMP PeerConnectionObserver::OnAddIceCandidateError(uint32_t name, const char * message)
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
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void onAddStream (in nsIDOMMediaStream stream); */
NS_IMETHODIMP PeerConnectionObserver::OnAddStream(nsIDOMMediaStream *stream)
{
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
NS_IMETHODIMP PeerConnectionObserver::FoundIceCandidate(const char * candidate)
{
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
}

PeerConnection::~PeerConnection()
{
}

void PeerConnection::Init_m() {
  _pc = sipcc::PeerConnectionImpl::CreatePeerConnection();
  sipcc::IceConfiguration cfg;

  _pc->Initialize(new PeerConnectionObserver(), nullptr, cfg,
                 PeerConnectionSingleton::Instance()->main_thread());
}

Handle<Value> PeerConnection::New( const Arguments& args ) {
  HandleScope scope;

  if( !args.IsConstructCall()) {
    return ThrowException(Exception::TypeError(
          String::New("Use the new operator to construct the PeerConnection.")));
  }

  PeerConnection* obj = new PeerConnection();
  obj->Wrap( args.This() );

  return args.This();
}

Handle<Value> PeerConnection::CreateOffer( const Arguments& args ) {
  HandleScope scope;

  INFO("PeerConnection::CreateOffer");
//  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );

  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::CreateAnswer( const Arguments& args ) {
  HandleScope scope;

  INFO("PeerConnection::CreateAnswer");

  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::SetLocalDescription( const Arguments& args ) {
  HandleScope scope;

  INFO("PeerConnection::SetLocalDescription");

  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::SetRemoteDescription( const Arguments& args ) {
  HandleScope scope;

  INFO("PeerConnection::SetRemoteDescription");

  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::AddIceCandidate( const Arguments& args ) {
  HandleScope scope;

  INFO("PeerConnection::AddIceCandidate");

  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::UpdateIce( const Arguments& args ) {
  HandleScope scope;

  INFO("PeerConnection::UpdateIce");

  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::Close( const Arguments& args ) {
  HandleScope scope;

  INFO("PeerConnection::Close");

  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::GetLocalDescription( Local<String> property, const AccessorInfo& info ) {
  HandleScope scope;

  INFO("PeerConnection::GetLocalDescription");

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );
  char* sdp = 0;
  self->_pc->GetLocalDescription(&sdp);
  
  Handle<Value> value;
  if(!sdp) {
    value = Null();
  } else {
    value = String::New(sdp);
  }

  return scope.Close(value);
}

Handle<Value> PeerConnection::GetRemoteDescription( Local<String> property, const AccessorInfo& info ) {
  HandleScope scope;

  INFO("PeerConnection::GetRemoteDescription");

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );
  char* sdp = 0;
  self->_pc->GetRemoteDescription(&sdp);
  
  Handle<Value> value;
  if(!sdp) {
    value = Null();
  } else {
    value = String::New(sdp);
  }

  return scope.Close(value);
}

Handle<Value> PeerConnection::GetSignalingState( Local<String> property, const AccessorInfo& info ) {
  HandleScope scope;

  INFO("PeerConnection::GetSignalingState");

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  uint32_t state;
  self->_pc->GetSignalingState(&state);

  return scope.Close(Number::New(state));
}

Handle<Value> PeerConnection::GetIceState( Local<String> property, const AccessorInfo& info ) {
  HandleScope scope;

  INFO("PeerConnection::GetIceState");

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( info.Holder() );

  uint32_t state;
  self->_pc->GetIceState(&state);

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
  tpl->InstanceTemplate()->SetAccessor(String::New("iceState"), GetSignalingState, ReadOnly);

  Persistent<Function> ctor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("PeerConnection"), ctor );
}
