/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <webrtc/api/scoped_refptr.h>
#include <webrtc/api/stats/rtc_stats_collector_callback.h>
#include <webrtc/api/stats/rtc_stats_report.h>

#include "src/events.h"

namespace node_webrtc {

class PeerConnection;

class RTCStatsCollector : public webrtc::RTCStatsCollectorCallback {
 public:
  RTCStatsCollector(
      PeerConnection* parent,
      std::unique_ptr<PromiseEvent<PeerConnection, rtc::scoped_refptr<webrtc::RTCStatsReport>>> promise)
    : _parent(parent)
    , _promise(std::move(promise)) {}

  void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>&) override;

 private:
  PeerConnection* _parent;
  std::unique_ptr<PromiseEvent<PeerConnection, rtc::scoped_refptr<webrtc::RTCStatsReport>>> _promise;
};

}  // namespace node_webrtc;
