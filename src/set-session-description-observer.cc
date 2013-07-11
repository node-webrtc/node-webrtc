#include "set-session-description-observer.h"

void SetLocalDescriptionObserver::OnSuccess() {
  parent->OnSetLocalDescriptionSuccess();
}
void SetLocalDescriptionObserver::OnFailure( const std::string& msg ) {
  parent->OnSetLocalDescriptionFailure( msg );
}

void SetRemoteDescriptionObserver::OnSuccess() {
  parent->OnSetRemoteDescriptionSuccess();
}
void SetRemoteDescriptionObserver::OnFailure( const std::string& msg ) {
  parent->OnSetRemoteDescriptionFailure( msg );
}