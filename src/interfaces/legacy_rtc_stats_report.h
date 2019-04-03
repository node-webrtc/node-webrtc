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
#include <string>

#include <node-addon-api/napi.h>

namespace node_webrtc {

class LegacyStatsReport
  : public Napi::ObjectWrap<LegacyStatsReport> {
 public:
  explicit LegacyStatsReport(const Napi::CallbackInfo&);

  ~LegacyStatsReport() = default;

  static void Init(Napi::Env, Napi::Object);

  static LegacyStatsReport* Create(double timestamp, const std::map<std::string, std::string>& stats);

 private:
  static Napi::FunctionReference& constructor();

  Napi::Value New(const Napi::CallbackInfo&);

  Napi::Value Names(const Napi::CallbackInfo&);
  Napi::Value Stat(const Napi::CallbackInfo&);

  Napi::Value GetTimestamp(const Napi::CallbackInfo&);
  Napi::Value GetType(const Napi::CallbackInfo&);

  double _timestamp;
  std::map<std::string, std::string> _stats;
};

}  // namespace node_webrtc
