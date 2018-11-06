/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcstatscollector.h"

#include "src/peerconnection.h"  // IWYU pragma: keep

// IWYU pragma: no_include <api/stats/rtcstatsreport.h>
// IWYU pragma: no_include <rtc_base/scoped_ref_ptr.h>
// IWYU pragma: no_forward_declare rtc::scoped_refptr
// IWYU pragma: no_forward_declare webrtc::RTCStatsReport

using node_webrtc::RTCStatsCollector;

void RTCStatsCollector::OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {
  if (_promise) {
    _promise->Resolve(report->Copy());
    _parent->Dispatch(std::move(_promise));
  }
}
