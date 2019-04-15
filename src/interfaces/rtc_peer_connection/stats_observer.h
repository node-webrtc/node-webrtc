/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <node-addon-api/napi.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/stats_types.h>

#include "src/interfaces/rtc_peer_connection.h"  // IWYU pragma: keep
#include "src/node/promise.h"

namespace node_webrtc {

class StatsObserver
  : public PromiseCreator<RTCPeerConnection>
  , public webrtc::StatsObserver {
 public:
  StatsObserver(
      RTCPeerConnection* peer_connection,
      Napi::Promise::Deferred deferred)
    : PromiseCreator(peer_connection, deferred) {}

  void OnComplete(const webrtc::StatsReports&) override;
};

}  // namespace node_webrtc
