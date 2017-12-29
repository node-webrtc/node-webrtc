/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
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
