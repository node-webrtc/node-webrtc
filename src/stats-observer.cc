/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/stats-observer.h"

#include <type_traits>

#include "src/converters.h"  // IWYU pragma: keep
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/converters/v8.h"  // IWYU pragma: keep
#include "src/error.h"
#include "src/events.h"
#include "src/functional/validation.h"
#include "src/peerconnection.h"

// IWYU pragma: no_include <api/scoped_refptr.h>
// IWYU pragma: no_include <api/stats_types.h>
// IWYU pragma: no_include <nan_implementation_12_inl.h>

node_webrtc::StatsObserver::StatsObserver(
    node_webrtc::PeerConnection* peer_connection)
  : _peer_connection(peer_connection) {
  Nan::HandleScope scope;
  _resolver = std::make_unique<Nan::Persistent<v8::Promise::Resolver>>(
          v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked());
}

node_webrtc::StatsObserver::StatsObserver(
    node_webrtc::PeerConnection* peer_connection,
    v8::Local<v8::Promise::Resolver> resolver)
  : _peer_connection(peer_connection) {
  Nan::HandleScope scope;
  _resolver = std::make_unique<Nan::Persistent<v8::Promise::Resolver>>(resolver);
}

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

  _peer_connection->Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>(
  [_resolver = std::move(_resolver), response]() {
    Nan::EscapableHandleScope scope;
    v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
    CONVERT_OR_REJECT_AND_RETURN(resolver, response, value, v8::Local<v8::Value>);
    resolver->Resolve(Nan::GetCurrentContext(), value).IsNothing();
  }));
}

v8::Local<v8::Promise> node_webrtc::StatsObserver::promise() {
  Nan::EscapableHandleScope scope;
  v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
  return scope.Escape(resolver->GetPromise());
}
