/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <iosfwd>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <nan.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/stats_types.h>
#include <v8.h>

namespace node_webrtc {

class PeerConnection;

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

class StatsObserver: public webrtc::StatsObserver {
 public:
  explicit StatsObserver(PeerConnection*);

  StatsObserver(PeerConnection*, v8::Local<v8::Promise::Resolver>);

  void OnComplete(const webrtc::StatsReports&) override;

  v8::Local<v8::Promise> promise();

 private:
  PeerConnection* _peer_connection;
  std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
};

}  // namespace node_webrtc
