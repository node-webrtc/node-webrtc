#include "create-session-description-observer.h"

void CreateSessionDescriptionObserver::OnSuccess( webrtc::SessionDescriptionInterface* sdp ) {
  parent->OnCreateSessionDescriptionSuccess( sdp );
}
void CreateSessionDescriptionObserver::OnFailure( const std::string& msg ) {
  parent->OnCreateSessionDescriptionFailure( msg );
}