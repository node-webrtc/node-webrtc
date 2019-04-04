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
#include <node-addon-api/napi.h>
#include <string>
#include <vector>

namespace node_webrtc {

class RTCStatsResponse
  : public Napi::ObjectWrap<RTCStatsResponse> {
 public:
  explicit RTCStatsResponse(const Napi::CallbackInfo&);

  static void Init(Napi::Env, Napi::Object);

  static RTCStatsResponse* Create(double timestamp, const std::vector<std::map<std::string, std::string>>& reports);

 private:
  static Napi::FunctionReference& constructor();

  Napi::Value Result(const Napi::CallbackInfo&);

  double _timestamp;
  std::vector<std::map<std::string, std::string>> _reports;
};

}  // namespace node_webrtc
