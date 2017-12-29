/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_STATS_OBSERVER_H_
#define SRC_STATS_OBSERVER_H_

#include "nan.h"  // IWYU pragma: keep

#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/statstypes.h"

namespace node_webrtc {

class PeerConnection;

class StatsObserver
: public webrtc::StatsObserver {
 private:
  PeerConnection* parent;
  Nan::Callback* callback;

 public:
  explicit StatsObserver(PeerConnection* parent, Nan::Callback *callback)
  : parent(parent), callback(callback) {}

  virtual void OnComplete(const webrtc::StatsReports& reports);
};

}  // namespace node_webrtc

#endif  // SRC_STATS_OBSERVER_H_
