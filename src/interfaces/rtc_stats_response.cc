/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_stats_response.h"

#include <cstdint>

#include <nan.h>
#include <v8.h>

#include "src/interfaces/legacy_rtc_stats_report.h"

Nan::Persistent<v8::Function>& node_webrtc::RTCStatsResponse::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

NAN_METHOD(node_webrtc::RTCStatsResponse::New) {
  if (info.Length() != 2 || !info[0]->IsExternal() || !info[1]->IsExternal()) {
    return Nan::ThrowTypeError("You cannot construct an RTCStatsResponse");
  }

  auto timestamp = static_cast<double*>(v8::Local<v8::External>::Cast(info[0])->Value());
  auto reports = static_cast<std::vector<std::map<std::string, std::string>>*>(v8::Local<v8::External>::Cast(info[1])->Value());

  auto obj = new node_webrtc::RTCStatsResponse(*timestamp, *reports);
  obj->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

NAN_METHOD(node_webrtc::RTCStatsResponse::result) {
  auto self = Nan::ObjectWrap::Unwrap<node_webrtc::RTCStatsResponse>(info.This());
  auto timestamp = self->_timestamp;
  auto reports = Nan::New<v8::Array>(self->_reports.size());

  uint32_t i = 0;
  for (auto const& stats : self->_reports) {
    auto report = LegacyStatsReport::Create(timestamp, stats);
    reports->Set(i++, report->handle());
  }

  info.GetReturnValue().Set(reports);
}

node_webrtc::RTCStatsResponse* node_webrtc::RTCStatsResponse::Create(
    double timestamp,
    const std::vector<std::map<std::string, std::string>>& reports) {
  Nan::HandleScope scope;
  v8::Local<v8::Value> cargv[2];
  cargv[0] = Nan::New<v8::External>(static_cast<void*>(&timestamp));
  cargv[1] = Nan::New<v8::External>(const_cast<void*>(static_cast<const void*>(&reports)));
  auto response = Nan::NewInstance(Nan::New(node_webrtc::RTCStatsResponse::constructor()), 2, cargv).ToLocalChecked();
  return Nan::ObjectWrap::Unwrap<node_webrtc::RTCStatsResponse>(response);
}

void node_webrtc::RTCStatsResponse::Init(v8::Handle<v8::Object> exports) {
  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate> (New);
  tpl->SetClassName(Nan::New("RTCStatsResponse").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
  Nan::SetPrototypeMethod(tpl, "result", result);
  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("RTCStatsResponse").ToLocalChecked(), tpl->GetFunction());
}
