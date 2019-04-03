/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/legacy_rtc_stats_report.h"

#include <cstdint>
#include <utility>
#include <vector>

#include <nan.h>
#include <v8.h>

#include "src/converters/arguments.h"
#include "src/converters/napi.h"
#include "src/functional/validation.h"

namespace node_webrtc {

Napi::FunctionReference& LegacyStatsReport::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

LegacyStatsReport::LegacyStatsReport(const Napi::CallbackInfo& info): Napi::ObjectWrap<LegacyStatsReport>(info) {
  if (info.Length() != 2 || !info[0].IsExternal() || !info[1].IsExternal()) {
    Napi::TypeError::New(info.Env(), "You cannot construct an LegacyStatsReport").ThrowAsJavaScriptException();
    return;
  }

  auto timestamp = static_cast<double*>(napi::UnsafeToV8(info[0]).As<v8::External>()->Value());
  auto stats = static_cast<std::map<std::string, std::string>*>(napi::UnsafeToV8(info[1]).As<v8::External>()->Value());

  _timestamp = *timestamp;
  _stats = *stats;
}

Napi::Value LegacyStatsReport::Names(const Napi::CallbackInfo& info) {
  auto names = std::vector<std::string>(_stats.size());
  uint64_t i = 0;
  for (const auto& stat : _stats) {
    names[i] = stat.first;
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), names, result, Napi::Value)
  return result;
}

Napi::Value LegacyStatsReport::Stat(const Napi::CallbackInfo& info) {
  CONVERT_ARGS_OR_THROW_AND_RETURN_NAPI(info, requested, std::string)

  Napi::Value found = info.Env().Undefined();
  for (const auto& pair : _stats) {
    auto name = pair.first;
    if (requested == name) {
      CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), pair.second, result, Napi::Value)
      found = result;
    }
  }

  return found;
}

Napi::Value LegacyStatsReport::GetTimestamp(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _timestamp, result, Napi::Value)
  return result;
}

Napi::Value LegacyStatsReport::GetType(const Napi::CallbackInfo& info) {
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), _stats.find("type")->second, result, Napi::Value)
  return result;
}

LegacyStatsReport* LegacyStatsReport::Create(double timestamp, const std::map<std::string, std::string>& stats) {
  auto env = LegacyStatsReport::constructor().Env();
  Napi::HandleScope scope(env);

  auto timestampExternal = Nan::New<v8::External>(static_cast<void*>(&timestamp));
  auto statsExternal = Nan::New<v8::External>(const_cast<void*>(static_cast<const void*>(&stats)));

  auto object = LegacyStatsReport::constructor().New({
    napi::UnsafeFromV8(env, timestampExternal),
    napi::UnsafeFromV8(env, statsExternal)
  });

  return LegacyStatsReport::Unwrap(object);
}

void LegacyStatsReport::Init(Napi::Env env, Napi::Object exports) {
  Napi::HandleScope scope(env);

  Napi::Function func = DefineClass(env, "LegacyStatsReport", {
    InstanceMethod("names", &LegacyStatsReport::Names),
    InstanceMethod("stat", &LegacyStatsReport::Stat),
    InstanceAccessor("timestamp", &LegacyStatsReport::GetTimestamp, nullptr),
    InstanceAccessor("type", &LegacyStatsReport::GetType, nullptr),
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("LegacyStatsReport", exports);
}

}  // namespace node_webrtc
