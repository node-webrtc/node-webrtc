/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCSTATSRESPONSE_H_
#define SRC_RTCSTATSRESPONSE_H_

#include "nan.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/statstypes.h"

#include "rtcstatsreport.h"

namespace node_webrtc {

class RTCStatsResponse
  : public Nan::ObjectWrap {
 public:
  explicit RTCStatsResponse(double timestamp, const std::vector<std::map<std::string, std::string>>& reports)
    : _timestamp(timestamp), _reports(reports) {}
  ~RTCStatsResponse() {}

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(result);

 private:
  double _timestamp;
  std::vector<std::map<std::string, std::string>> _reports;
};

}  // namespace node_webrtc

#endif  // SRC_RTCSTATSRESPONSE_H_
