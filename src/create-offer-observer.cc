#include "create-offer-observer.h"
#include "peerconnection.h"
#include "common.h"

using namespace node_webrtc;

void CreateOfferObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp)
{
  TRACE_CALL;
  PeerConnection::SdpEvent* data = new PeerConnection::SdpEvent(sdp);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}

void CreateOfferObserver::OnFailure(const std::string& msg)
{
  TRACE_CALL;
  PeerConnection::ErrorEvent* data = new PeerConnection::ErrorEvent(msg);
  parent->QueueEvent(PeerConnection::CREATE_OFFER_ERROR, (void*)data);
  TRACE_END;
}