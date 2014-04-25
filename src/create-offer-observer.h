#include "talk/app/webrtc/peerconnectioninterface.h"

namespace node_webrtc {

class PeerConnection;

class CreateOfferObserver
  : public webrtc::CreateSessionDescriptionObserver
{
  private:
    PeerConnection* parent;

  public:
    CreateOfferObserver( PeerConnection* connection ): parent(connection) {};

    virtual void OnSuccess( webrtc::SessionDescriptionInterface* sdp );
    virtual void OnFailure( const std::string& msg );
};

}