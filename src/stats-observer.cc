#include <iostream>

#include "common.h"
#include "peerconnection.h"
#include "stats-observer.h"

using namespace node_webrtc;

void StatsObserver::OnComplete(const webrtc::StatsReports& reports)
{
  TRACE_CALL;
  webrtc::StatsReports copy = reports;
  PeerConnection::GetStatsEvent* data = new PeerConnection::GetStatsEvent(this->callback, copy);
  parent->QueueEvent(PeerConnection::GET_STATS_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}
