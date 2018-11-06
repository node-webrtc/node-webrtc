/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_STATS_OBSERVER_H_
#define SRC_STATS_OBSERVER_H_

#include <webrtc/api/peerconnectioninterface.h>
#include <webrtc/api/statstypes.h>

#include "src/events.h"  // IWYU pragma: keep

namespace node_webrtc {

class PeerConnection;

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

class StatsObserver
  : public webrtc::StatsObserver {
 private:
  PeerConnection* parent;
  std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection, node_webrtc::RTCStatsResponseInit>> _promise;

 public:
  StatsObserver(
      PeerConnection* parent,
      std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection, node_webrtc::RTCStatsResponseInit>> promise)
    : parent(parent), _promise(std::move(promise)) {}

  virtual void OnComplete(const webrtc::StatsReports& reports);
};

}  // namespace node_webrtc

#endif  // SRC_STATS_OBSERVER_H_
