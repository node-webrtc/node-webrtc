#include "talk/app/webrtc/peerconnectioninterface.h"

class PeerConnection;

// CreateSessionDescriptionObserver is required for Jsep callbacks.
class CreateSessionDescriptionObserver :
  public webrtc::CreateSessionDescriptionObserver
{
  private:
    PeerConnection* parent;
  public:
    CreateSessionDescriptionObserver( PeerConnection* connection ): parent(connection) {};

    virtual void OnSuccess( webrtc::SessionDescriptionInterface* sdp );
    virtual void OnFailure( const std::string& msg );
};