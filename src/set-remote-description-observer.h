#include "talk/app/webrtc/peerconnectioninterface.h"

class PeerConnection;

class SetRemoteDescriptionObserver :
  public webrtc::SetSessionDescriptionObserver
{
  private:
    PeerConnection* parent;
  public:
    SetRemoteDescriptionObserver( PeerConnection* connection): parent(connection) {};

    virtual void OnSuccess();
    virtual void OnFailure( const std::string& msg );
};