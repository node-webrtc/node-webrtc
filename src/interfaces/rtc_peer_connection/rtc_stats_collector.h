/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <node-addon-api/napi.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/api/stats/rtc_stats_collector_callback.h>

#include "src/interfaces/rtc_peer_connection.h"  // IWYU pragma: keep
#include "src/node/promise.h"

namespace webrtc { class RTCStatsReport; }

namespace node_webrtc {

class RTCStatsCollector
  : public PromiseCreator<RTCPeerConnection>
  , public webrtc::RTCStatsCollectorCallback {
 public:
  RTCStatsCollector(
      RTCPeerConnection* peer_connection,
      Napi::Promise::Deferred deferred)
    : PromiseCreator<RTCPeerConnection>(peer_connection, deferred) {}

  void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>&) override;
};

}  // namespace node_webrtc;
