#include "set-local-description-observer.h"

#include "common.h"
#include "peerconnection.h"

using node_webrtc::PeerConnection;
using node_webrtc::SetLocalDescriptionObserver;

void SetLocalDescriptionObserver::OnSuccess() {
  TRACE_CALL;
  parent->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_SUCCESS, static_cast<void*>(nullptr));
  TRACE_END;
}

void SetLocalDescriptionObserver::OnFailure(const std::string& msg) {
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::SET_LOCAL_DESCRIPTION_ERROR, static_cast<void*>(data));
  TRACE_END;
}
