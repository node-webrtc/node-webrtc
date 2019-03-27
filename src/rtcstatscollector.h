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
#include <v8.h>

#include "src/peerconnection.h"  // IWYU pragma: keep
#include "src/promise.h"

namespace webrtc { class RTCStatsReport; }

namespace node_webrtc {

class RTCStatsCollector
  : public PromiseCreator<PeerConnection>
  , public webrtc::RTCStatsCollectorCallback {
 public:
  RTCStatsCollector(
      PeerConnection* peer_connection,
      v8::Local<v8::Promise::Resolver> resolver)
    : PromiseCreator<PeerConnection>(peer_connection, resolver) {}

  void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>&) override;
};

}  // namespace node_webrtc;
