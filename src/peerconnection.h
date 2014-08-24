#ifndef __PEERCONNECTION_H__
#define __PEERCONNECTION_H__

#include <queue>
#include <string>

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/ref_count.h"
#include "datachannel.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

namespace node_webrtc {

class CreateOfferObserver;
class CreateAnswerObserver;
class SetLocalDescriptionObserver;
class SetRemoteDescriptionObserver;

class PeerConnection
: public ObjectWrap,
  public webrtc::PeerConnectionObserver
{

public:

  struct ErrorEvent {
    ErrorEvent(const std::string& msg)
    : msg(msg) {}

    std::string msg;
  };

  struct SdpEvent
  {
    SdpEvent(webrtc::SessionDescriptionInterface* sdp)
    {
      if(!sdp->ToString(&desc))
      {
        desc = "";
      }
      type = sdp->type();
    }

    std::string type;
    std::string desc;
  };

  struct IceEvent {
    IceEvent(const webrtc::IceCandidateInterface* ice_candidate)
    : sdpMLineIndex(ice_candidate->sdp_mline_index()),
      sdpMid(ice_candidate->sdp_mid())
    {
      ice_candidate->ToString(&candidate);
    }

    uint32_t sdpMLineIndex;
    std::string sdpMid;
    std::string candidate;
  };

  struct StateEvent {
    StateEvent(uint32_t state)
    : state(state) {}

    uint32_t state;
  };

  struct DataChannelEvent {
    DataChannelEvent(DataChannelObserver* observer)
    : observer(observer) {};

    DataChannelObserver* observer;
  };

  enum AsyncEventType {
    CREATE_OFFER_SUCCESS = 0x1 << 0, // 1
    CREATE_OFFER_ERROR = 0x1 << 1, // 2
    CREATE_ANSWER_SUCCESS = 0x1 << 2, // 4
    CREATE_ANSWER_ERROR = 0x1 << 3, // 8
    SET_LOCAL_DESCRIPTION_SUCCESS = 0x1 << 4, // 16
    SET_LOCAL_DESCRIPTION_ERROR = 0x1 << 5, // 32
    SET_REMOTE_DESCRIPTION_SUCCESS = 0x1 << 6, // 64
    SET_REMOTE_DESCRIPTION_ERROR = 0x1 << 7, // 128
    ADD_ICE_CANDIDATE_SUCCESS = 0x1 << 8, // 256
    ADD_ICE_CANDIDATE_ERROR = 0x1 << 9, // 512
    NOTIFY_DATA_CHANNEL = 0x1 << 10, // 1024
    NOTIFY_CONNECTION = 0x1 << 11, // 2048
    NOTIFY_CLOSED_CONNECTION = 0x1 << 12, // 4096
    ICE_CANDIDATE = 0x1 << 13, // 8192
    SIGNALING_STATE_CHANGE = 0x1 << 14, // 16384
    ICE_CONNECTION_STATE_CHANGE = 0x1 << 15, // 32768
    ICE_GATHERING_STATE_CHANGE = 0x1 << 16, // 65536
    NOTIFY_ADD_STREAM = 0x1 << 17, // 131072
    NOTIFY_REMOVE_STREAM = 0x1 << 18, // 262144

    ERROR_EVENT = CREATE_OFFER_ERROR | CREATE_ANSWER_ERROR |
                  SET_LOCAL_DESCRIPTION_ERROR | SET_REMOTE_DESCRIPTION_ERROR |
                  ADD_ICE_CANDIDATE_ERROR,
    SDP_EVENT = CREATE_OFFER_SUCCESS | CREATE_ANSWER_SUCCESS,
    VOID_EVENT = SET_LOCAL_DESCRIPTION_SUCCESS | SET_REMOTE_DESCRIPTION_SUCCESS |
                 ADD_ICE_CANDIDATE_SUCCESS,
    STATE_EVENT = SIGNALING_STATE_CHANGE | ICE_CONNECTION_STATE_CHANGE |
                  ICE_GATHERING_STATE_CHANGE
  };

  PeerConnection();
  ~PeerConnection();

  //
  // PeerConnectionObserver implementation.
  //

  virtual void OnError();

  virtual void OnSignalingChange( webrtc::PeerConnectionInterface::SignalingState new_state );
  virtual void OnIceConnectionChange( webrtc::PeerConnectionInterface::IceConnectionState new_state );
  virtual void OnIceGatheringChange( webrtc::PeerConnectionInterface::IceGatheringState new_state );
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate );
  virtual void OnRenegotiationNeeded();

  virtual void OnAddStream( webrtc::MediaStreamInterface* stream );
  virtual void OnRemoveStream( webrtc::MediaStreamInterface* stream );
  virtual void OnDataChannel( webrtc::DataChannelInterface* data_channel );

  //
  // Nodejs wrapping.
  //
  static void Init( Handle<Object> exports );
  static Persistent<Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(CreateOffer);
  static NAN_METHOD(CreateAnswer);
  static NAN_METHOD(SetLocalDescription);
  static NAN_METHOD(SetRemoteDescription);
  static NAN_METHOD(UpdateIce);
  static NAN_METHOD(AddIceCandidate);
  static NAN_METHOD(CreateDataChannel);

  static NAN_METHOD(GetLocalStreams);
  static NAN_METHOD(GetRemoteStreams);
  static NAN_METHOD(GetStreamById);
  static NAN_METHOD(AddStream);
  static NAN_METHOD(RemoveStream);

  static NAN_METHOD(Close);

  static NAN_GETTER(GetLocalDescription);
  static NAN_GETTER(GetRemoteDescription);
  static NAN_GETTER(GetIceConnectionState);
  static NAN_GETTER(GetSignalingState);
  static NAN_GETTER(GetIceGatheringState);
  static NAN_SETTER(ReadOnly);

  void QueueEvent(AsyncEventType type, void* data);

private:
  static void Run(uv_async_t* handle, int status);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  uv_loop_t *loop;
  std::queue<AsyncEvent> _events;
  webrtc::PeerConnectionInterface::IceServers _iceServers;
  webrtc::MediaConstraintsInterface* _mediaConstraints;

  talk_base::scoped_refptr<CreateOfferObserver> _createOfferObserver;
  talk_base::scoped_refptr<CreateAnswerObserver> _createAnswerObserver;
  talk_base::scoped_refptr<SetLocalDescriptionObserver> _setLocalDescriptionObserver;
  talk_base::scoped_refptr<SetRemoteDescriptionObserver> _setRemoteDescriptionObserver;

  talk_base::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _jinglePeerConnectionFactory;
  talk_base::scoped_refptr<webrtc::PeerConnectionInterface> _jinglePeerConnection;
};

}

#endif
