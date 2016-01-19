#include "create-answer-observer.h"

#include "common.h"
#include "peerconnection.h"

using node_webrtc::CreateAnswerObserver;
using node_webrtc::PeerConnection;

void CreateAnswerObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp) {
  TRACE_CALL;
  PeerConnection::SdpEvent* data = new PeerConnection::SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateAnswerObserver::OnFailure(const std::string& msg) {
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_ANSWER_ERROR, static_cast<void*>(data));
  TRACE_END;
}
