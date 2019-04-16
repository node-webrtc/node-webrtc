/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_stats_response.h"

#include <nan.h>
#include <v8.h>

#include "src/converters/napi.h"
#include "src/functional/validation.h"
#include "src/interfaces/legacy_rtc_stats_report.h"

namespace node_webrtc {

Napi::FunctionReference& RTCStatsResponse::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

RTCStatsResponse::RTCStatsResponse(const Napi::CallbackInfo& info): Napi::ObjectWrap<RTCStatsResponse>(info) {
  auto env = info.Env();
  Napi::HandleScope scope(env);

  if (info.Length() != 2 || !info[0].IsExternal() || !info[1].IsExternal()) {
    Napi::TypeError::New(env, "You cannot construct an RTCStatsResponse").ThrowAsJavaScriptException();
    return;
  }

  auto timestamp = static_cast<double*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[0]))->Value());
  auto reports = static_cast<std::vector<std::map<std::string, std::string>>*>(v8::Local<v8::External>::Cast(napi::UnsafeToV8(info[1]))->Value());

  _timestamp = *timestamp;
  _reports = *reports;
}

Napi::Value RTCStatsResponse::Result(const Napi::CallbackInfo& info) {
  auto reports = std::vector<Napi::Value>();
  reports.reserve(_reports.size());
  for (auto const& stats : _reports) {
    reports.push_back(LegacyStatsReport::Create(_timestamp, stats)->Value());
  }
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), reports, result, Napi::Value)
  return result;
}

RTCStatsResponse* RTCStatsResponse::Create(
    double timestamp,
    const std::vector<std::map<std::string, std::string>>& reports) {
  auto env = RTCStatsResponse::constructor().Env();
  Napi::HandleScope scope(env);

  auto timestampExternal = Nan::New<v8::External>(static_cast<void*>(&timestamp));
  auto reportsExternal = Nan::New<v8::External>(const_cast<void*>(static_cast<const void*>(&reports)));

  auto response = RTCStatsResponse::constructor().New({
    napi::UnsafeFromV8(env, timestampExternal),
    napi::UnsafeFromV8(env, reportsExternal)
  });

  return RTCStatsResponse::Unwrap(response);
}

void RTCStatsResponse::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(env, "RTCStatsResponse", {
    InstanceMethod("result", &RTCStatsResponse::Result)
  });

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCStatsResponse", func);
}

}  // namespace node_webrtc
