/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/stats_types.h>
#include <v8.h>

#include "src/interfaces/peerconnection.h"  // IWYU pragma: keep
#include "src/promise.h"

namespace node_webrtc {

class StatsObserver
  : public PromiseCreator<PeerConnection>
  , public webrtc::StatsObserver {
 public:
  StatsObserver(
      PeerConnection* peer_connection,
      v8::Local<v8::Promise::Resolver> resolver)
    : PromiseCreator(peer_connection, resolver) {}

  void OnComplete(const webrtc::StatsReports&) override;
};

}  // namespace node_webrtc
