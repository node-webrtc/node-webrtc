/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <iosfwd>
#include <memory>
#include <map>
#include <utility>
#include <vector>

#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/stats_types.h>

#include "src/events.h"

namespace node_webrtc {

class PeerConnection;

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

class StatsObserver
  : public webrtc::StatsObserver {
 private:
  PeerConnection* parent;
  std::unique_ptr<PromiseEvent<PeerConnection, RTCStatsResponseInit>> _promise;

 public:
  StatsObserver(
      PeerConnection* parent,
      std::unique_ptr<PromiseEvent<PeerConnection, RTCStatsResponseInit>> promise)
    : parent(parent), _promise(std::move(promise)) {}

  virtual void OnComplete(const webrtc::StatsReports& reports);
};

}  // namespace node_webrtc
