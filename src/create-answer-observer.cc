#include "create-answer-observer.h"
#include "peerconnection.h"
#include "common.h"

using namespace node_webrtc;

void CreateAnswerObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  PeerConnection::SdpEvent* data = new PeerConnection::SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateAnswerObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_ERROR, (void*)data);
  TRACE_END;
}