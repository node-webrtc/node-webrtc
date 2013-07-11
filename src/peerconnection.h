#include <v8.h>
#include <node.h>
#include <uv.h>

#include <queue>

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "third_party/webrtc/system_wrappers/interface/ref_count.h"

using namespace node;
using namespace v8;

class PeerConnection :
  public ObjectWrap,

  // PeerConnectionObserver implements the main PeerConnection handling.
  public webrtc::PeerConnectionObserver
{

private:

  webrtc::RefCountImpl<CallbackAudioDevice>* _audioDevice;
  webrtc::PeerConnectionInterface::IceServers _iceServers;

  talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _peerConnectionFactory;
  talk_base::scoped_refptr<webrtc::PeerConnectionInterface> _peerConnection;

  talk_base::Thread* _signalThread;
  talk_base::Thread* _workerThread;

  talk_base::scoped_refptr<CreateSessionDescriptionObserver> _createSessionDescriptionObserver;
  talk_base::scoped_refptr<SetRemoteDescriptionObserver> _setRemoteDescriptionObserver;

  uv_mutex_t eventLock;
  uv_async_t emitAsync;;

public:

  PeerConnection();
  ~PeerConnection();

  void SetPeerConnection( webrtc::PeerConnectionInterface* peerConnection );

  void OnCreateSessionDescriptionSuccess( webrtc::SessionDescriptionInterface* sdp );
  void OnCreateSessionDescriptionFailure( const std::string& msg );
  void OnSetLocalDescriptionSuccess();
  void OnSetLocalDescriptionFailure( const std::string& msg );
  void OnSetRemoteDescriptionSuccess();
  void OnSetRemoteDescriptionFailure( const std::string& msg );

  //
  // PeerConnectionObserver implementation.
  //

  virtual void OnError();

  // Triggered when SignalingState changed.
  virtual void OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState new_state );

  // New Ice candidate have been found.
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate );

  // Called any time the ICEConnectionState changes
  virtual void OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState new_state );

  // Called any time the ICEGatheringState changes
  virtual void OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState new_state );

  // Triggered when media is received on a new stream from remote peer.
  virtual void OnAddStream( webrtc::MediaStreamInterface* stream );

  // Triggered when a remote peer close a stream.
  virtual void OnRemoveStream( webrtc::MediaStreamInterface* stream );

  virtual void OnRenegotiationNeeded();

  virtual void OnDataChannel( webrtc::DataChannelInterface* data_channel );

  //
  // Nodejs wrapping.
  //
  static void Init( Handle<Object> exports );
  static Persistent<Function> constructor;
  static Handle<Value> New( const Arguments& args );

  static Handle<Value> CreateOffer( const Arguments& args );
  static Handle<Value> CreateAnswer( const Arguments& args );
  static Handle<Value> SetLocalDescription( const Arguments& args );
  static Handle<Value> SetRemoteDescription( const Arguments& args );
  static Handle<Value> UpdateIce( const Arguments& args );
  static Handle<Value> AddIceCandidate( const Arguments& args );
  static Handle<Value> Close( const Arguments& args );

};