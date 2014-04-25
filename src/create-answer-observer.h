#include "talk/app/webrtc/peerconnectioninterface.h"

namespace node_webrtc {

class PeerConnection;

class CreateAnswerObserver
  : public webrtc::CreateSessionDescriptionObserver
{
  private:
    PeerConnection* parent;

  public:
    CreateAnswerObserver( PeerConnection* connection ): parent(connection) {};

    virtual void OnSuccess( webrtc::SessionDescriptionInterface* sdp );
    virtual void OnFailure( const std::string& msg );
};

}