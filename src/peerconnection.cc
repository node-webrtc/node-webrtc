
#include <node_buffer.h>

#include "peerconnection.h"
#include "talk/app/webrtc/jsep.h"
#include "third_party/webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"

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

Persistent<Function> PeerConnection::constructor;

//
// PeerConnection
//

PeerConnection::PeerConnection()
{
  // Init the uvlib stuff.
  uv_mutex_init( &eventLock );
  uv_async_init( uv_default_loop(), &emitAsync, EmitAsync );

  // Create the observers.
  _createSessionDescriptionObserver = new talk_base::RefCountedObject<CreateSessionDescriptionObserver>( this );
  _setRemoteDescriptionObserver = new talk_base::RefCountedObject<SetRemoteDescriptionObserver>( this );

  // Init the PeerConnectionFactory
  _signalThread = new talk_base::Thread;
  _workerThread = new talk_base::Thread;

  _signalThread->Start();
  _workerThread->Start();

  _peerConnectionFactory = webrtc::CreatePeerConnectionFactory(
      _signalThread, _workerThread, NULL, NULL, NULL );

  // Create the PeerConnection
  _peerConnection =
    _peerConnectionFactory->CreatePeerConnection( _iceServers, NULL, this );
}

PeerConnection::~PeerConnection()
{
  delete _signalThread;
  delete _workerThread;
}

void PeerConnection::SetPeerConnection( webrtc::PeerConnectionInterface* peerConnection ) {
  ASSERT( peerConnection );
  ASSERT( _peerConnection.get() == NULL );
  _peerConnection = peerConnection;
}

void PeerConnection::OnCreateSessionDescriptionSuccess( webrtc::SessionDescriptionInterface* sdp ) {
  // TODO: run successCallback for create{Offer,Answer}
}

void PeerConnection::OnCreateSessionDescriptionFailure( const std::string& msg ) {
  // TODO: run failureCallback for create{Offer,Answer}
}

void PeerConnection::OnSetLocalDescriptionSuccess() {
}

void PeerConnection::OnSetLocalDescriptionFailure( const std::string& msg ) {
}

void PeerConnection::OnSetRemoteDescriptionSuccess() {
}

void PeerConnection::OnSetRemoteDescriptionFailure( const std::string& msg ) {
}

void PeerConnection::OnError() {
  // TODO: run error callback.
}

void PeerConnection::OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState new_state ) {
  // TODO: run callback.
}

void PeerConnection::OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState new_state ) {
  // TODO: run callback.
}

void PeerConnection::OnIceCandidate( const webrtc::IceCandidateInterface* candidate ) {
  // TODO: run callback.
}

void PeerConnection::OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState new_state ) {
  // TODO: run callback.
}

void PeerConnection::OnAddStream( webrtc::MediaStreamInterface* stream ) {
}

void PeerConnection::OnRemoveStream( webrtc::MediaStreamInterface* stream ) {
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

}

Handle<Value> PeerConnection::CreateAnswer( const Arguments& args ) {

}

Handle<Value> PeerConnection::SetLocalDescription( const Arguments& args ) {

}

Handle<Value> PeerConnection::SetRemoteDescription( const Arguments& args ) {
  HandleScope scope;

  REQ_OBJ_ARG( 0, desc );

  String::Utf8Value type( desc->Get( String::NewSymbol( "type" ) )->ToString() );
  String::Utf8Value sdp( desc->Get( String::NewSymbol( "sdp" ) )->ToString() );

  webrtc::SessionDescriptionInterface* session_description(
      webrtc::CreateSessionDescription(
          std::string( *type ),
          std::string( *sdp )));

  if( !session_description ) {
    ThrowException( Exception::TypeError( String::New("Could not parse session description") ) );
    return scope.Close(Undefined());
  };

  PeerConnection* rtcConnection = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  rtcConnection->SetRemoteDescription( session_description );

  return scope.Close( Undefined() );
}


Handle<Value> PeerConnection::AddIceCandidate( const Arguments& args ) {
  HandleScope scope;

  REQ_OBJ_ARG( 0, iceCandidate );

  String::Utf8Value sdpMid( iceCandidate->Get( String::NewSymbol( "sdpMid" ) )->ToString() );
  String::Utf8Value candidate( iceCandidate->Get( String::NewSymbol( "candidate" ) )->ToString() );
  int sdpMLineIndex( iceCandidate->Get( String::NewSymbol( "sdpMLineIndex" ) )->ToNumber()->IntegerValue() );

  talk_base::scoped_ptr< webrtc::IceCandidateInterface > iceItf(
      webrtc::CreateIceCandidate(
          std::string( *sdpMid ),
          sdpMLineIndex,
          std::string( *candidate )
      ));

  if( !iceItf.get() ) {
    return ThrowException( Exception::TypeError(
          String::New("Failed to parse candidate!" )));
  }


  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  if( !self->_peerConnection->AddIceCandidate( iceItf.get() ) ) {
    return ThrowException( Exception::TypeError(
          String::New("Failed to add candidate!" )));
  }

  return scope.Close( Undefined() );
}

Handle<Value> PeerConnection::UpdateIce( const Arguments& args ) {

}

Handle<Value> PeerConnection::Close( const Arguments& args ) {
  HandleScope scope;

  PeerConnection* self = ObjectWrap::Unwrap<PeerConnection>( args.This() );
  self->_peerConnection->Close();

  return scope.Close( Undefined() );
}

void PeerConnection::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "PeerConnection" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createOffer" ),
    FunctionTemplate::New( CreateOffer )->GetFunction() );

  tpl->PrototypeTemplate()->Set( String::NewSymbol( "createAnswer" ),
    FunctionTemplate::New( createAnswer )->GetFunction() );

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

  Persistent<Function> ctor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("PeerConnection"), ctor );
}
