/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_peer_connection/stats_observer.h"

#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <webrtc/api/scoped_refptr.h>

#include "src/dictionaries/node_webrtc/rtc_stats_response_init.h"  // IWYU pragma: keep

void node_webrtc::StatsObserver::OnComplete(const webrtc::StatsReports& stats_reports) {
  double timestamp = 0;
  auto reports = std::vector<std::map<std::string, std::string>>();
  for (auto stats_report : stats_reports) {
    auto report = std::map<std::string, std::string>();
    // NOTE(mroberts): This is a little janky. We should thread each report's timestamp along.
    timestamp = timestamp > stats_report->timestamp() ? timestamp : stats_report->timestamp();
    report.emplace("type", stats_report->TypeToString());
    for (auto const& pair : stats_report->values()) {
      auto stat = std::string(pair.second->display_name());
      auto value = std::string(pair.second->ToString());
      report.emplace(stat, value);
    }
    reports.push_back(report);
  }

  std::pair<double, std::vector<std::map<std::string, std::string>>> response(timestamp, reports);

  Resolve(response);
}
