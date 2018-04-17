/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
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

void StatsObserver::OnComplete(const webrtc::StatsReports& statsReports) {
  TRACE_CALL;
  if (_promise) {
    double timestamp = 0;
    auto reports = std::vector<std::map<std::string, std::string>>();
    for (auto statsReport : statsReports) {
      auto report = std::map<std::string, std::string>();
      // NOTE(mroberts): This is a little janky. We should thread each report's timestamp along.
      timestamp = timestamp > statsReport->timestamp() ? timestamp : statsReport->timestamp();
      report.emplace("type", statsReport->TypeToString());
      for (auto const& pair : statsReport->values()) {
        auto stat = std::string(pair.second->display_name());
        auto value = std::string(pair.second->ToString());
        report.emplace(stat, value);
      }
      reports.push_back(report);
    }
    _promise->Resolve(std::pair<double, std::vector<std::map<std::string, std::string>>>(timestamp, reports));
    parent->Dispatch(std::move(_promise));
  }
  TRACE_END;
}
