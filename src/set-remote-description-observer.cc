#include "set-remote-description-observer.h"
#include "peerconnection.h"
#include "common.h"

using namespace node_webrtc;

void SetRemoteDescriptionObserver::OnSuccess()
{
  TRACE_CALL;
  parent->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_SUCCESS, static_cast<void*>(NULL));
  TRACE_END;
}

void SetRemoteDescriptionObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::SET_REMOTE_DESCRIPTION_ERROR, (void*)data);
  TRACE_END;
}