#include "talk/app/webrtc/peerconnectioninterface.h"

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