#include "stats-observer.h"

#include "common.h"
#include "peerconnection.h"

using node_webrtc::PeerConnection;
using node_webrtc::StatsObserver;

void StatsObserver::OnComplete(const webrtc::StatsReports& reports) {
  TRACE_CALL;
  webrtc::StatsReports copy = reports;
  PeerConnection::GetStatsEvent* data = new PeerConnection::GetStatsEvent(this->callback, copy);
  parent->QueueEvent(PeerConnection::GET_STATS_SUCCESS, static_cast<void*>(data));
  TRACE_END;
}
