/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCSTATSREPORT_H_
#define SRC_RTCSTATSREPORT_H_

#include "nan.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/statstypes.h"  // IWYU pragma: keep

namespace node_webrtc {

class RTCStatsReport
  : public Nan::ObjectWrap {
 public:
  explicit RTCStatsReport(double timestamp, const std::map<std::string, std::string>& stats)
    : _timestamp(timestamp), _stats(stats) {}
  ~RTCStatsReport() {}

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(names);
  static NAN_METHOD(stat);

  static NAN_GETTER(GetTimestamp);
  static NAN_GETTER(GetType);

  static NAN_SETTER(ReadOnly);

 private:
  double _timestamp;
  std::map<std::string, std::string> _stats;
};

}  // namespace node_webrtc

#endif  // SRC_RTCSTATSREPORT_H_
