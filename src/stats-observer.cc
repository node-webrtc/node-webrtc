/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/stats-observer.h"

#include <iosfwd>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <nan.h>

#include "src/converters.h"  // IWYU pragma: keep
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/converters/v8.h"  // IWYU pragma: keep
#include "src/error.h"
#include "src/functional/validation.h"

// IWYU pragma: no_include <api/scoped_refptr.h>
// IWYU pragma: no_include <api/stats_types.h>
// IWYU pragma: no_include <nan_implementation_12_inl.h>
// IWYU pragma: no_include "src/events.h"

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

  Dispatch([response](v8::Local<v8::Promise::Resolver> resolver) {
    Nan::EscapableHandleScope scope;
    CONVERT_OR_REJECT_AND_RETURN(resolver, response, value, v8::Local<v8::Value>);
    resolver->Resolve(Nan::GetCurrentContext(), value).IsNothing();
  });
}
