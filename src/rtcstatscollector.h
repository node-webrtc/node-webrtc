/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCSTATSCOLLECTOR_H_
#define SRC_RTCSTATSCOLLECTOR_H_

#include <webrtc/api/stats/rtcstatscollectorcallback.h>
#include <webrtc/api/stats/rtcstatsreport.h>  // IWYU pragma: keep
#include <webrtc/rtc_base/scoped_ref_ptr.h>  // IWYU pragma: keep

#include "src/events.h"  // IWYU pragma: keep

namespace node_webrtc {

class PeerConnection;

class RTCStatsCollector : public webrtc::RTCStatsCollectorCallback {
 public:
  RTCStatsCollector(
      PeerConnection* parent,
      std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection, rtc::scoped_refptr<webrtc::RTCStatsReport>>> promise)
    : _parent(parent)
    , _promise(std::move(promise)) {}

  void OnStatsDelivered(const rtc::scoped_refptr<const webrtc::RTCStatsReport>&) override;

 private:
  PeerConnection* _parent;
  std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection, rtc::scoped_refptr<webrtc::RTCStatsReport>>> _promise;
};

}  // namespace node_webrtc;

#endif  // SRC_RTCSTATSCOLLECTOR_H_
