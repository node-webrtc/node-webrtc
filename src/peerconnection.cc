
#include <node_buffer.h>

#include "common.h"
#include "peerconnection.h"

using namespace node;
using namespace v8;

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

//
// PeerConnection
//

PeerConnection::PeerConnection()
{
}

PeerConnection::~PeerConnection()
{
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
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::CreateAnswer( const Arguments& args ) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::SetLocalDescription( const Arguments& args ) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::SetRemoteDescription( const Arguments& args ) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::AddIceCandidate( const Arguments& args ) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::UpdateIce( const Arguments& args ) {
  HandleScope scope;
  return scope.Close(Undefined());
}

Handle<Value> PeerConnection::Close( const Arguments& args ) {
  HandleScope scope;
  return scope.Close(Undefined());
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

  Persistent<Function> ctor = Persistent<Function>::New( tpl->GetFunction() );
  exports->Set( String::NewSymbol("PeerConnection"), ctor );
}
