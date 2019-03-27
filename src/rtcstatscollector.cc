/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/rtcstatscollector.h"

#include <nan.h>
#include <webrtc/api/stats/rtc_stats_report.h>

#include "src/converters.h"  // IWYU pragma: keep
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/error.h"
#include "src/functional/validation.h"

// IWYU pragma: no_include <api/scoped_refptr.h>
// IWYU pragma: no_include <nan_implementation_12_inl.h>
// IWYU pragma: no_include "src/events.h"

void node_webrtc::RTCStatsCollector::OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>& report) {
  Dispatch([report = report->Copy()](v8::Local<v8::Promise::Resolver> resolver) {
    Nan::HandleScope scope;
    CONVERT_OR_REJECT_AND_RETURN(resolver, report, value, v8::Local<v8::Value>);
    resolver->Resolve(Nan::GetCurrentContext(), value).IsNothing();
  });
}
