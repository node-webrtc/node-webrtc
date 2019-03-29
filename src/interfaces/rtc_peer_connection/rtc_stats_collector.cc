/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_peer_connection/rtc_stats_collector.h"

#include <webrtc/api/stats/rtc_stats_report.h>

#include "src/dictionaries/webrtc/rtc_stats_report.h"

// IWYU pragma: no_include <api/scoped_refptr.h>
// IWYU pragma: no_include <nan_implementation_12_inl.h>
// IWYU pragma: no_include "src/events.h"

void node_webrtc::RTCStatsCollector::OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {
  Resolve(report->Copy());
}
