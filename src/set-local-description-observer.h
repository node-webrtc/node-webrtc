#include "talk/app/webrtc/peerconnectioninterface.h"

namespace node_webrtc {

class PeerConnection;

class SetLocalDescriptionObserver :
  public webrtc::SetSessionDescriptionObserver
{
  private:
    PeerConnection* parent;
  public:
    SetLocalDescriptionObserver( PeerConnection* connection): parent(connection) {};

    virtual void OnSuccess();
    virtual void OnFailure( const std::string& msg );
};

}