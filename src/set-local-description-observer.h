/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_SET_LOCAL_DESCRIPTION_OBSERVER_H_
#define SRC_SET_LOCAL_DESCRIPTION_OBSERVER_H_

#include <string>

#include "webrtc/api/jsep.h"

namespace node_webrtc {

class PeerConnection;

class SetLocalDescriptionObserver
  : public webrtc::SetSessionDescriptionObserver {
 private:
  PeerConnection* parent;

 public:
  explicit SetLocalDescriptionObserver(PeerConnection* connection): parent(connection) {}

  virtual void OnSuccess();
  virtual void OnFailure(const std::string& msg);
};

}  // namespace node_webrtc

#endif  // SRC_SET_LOCAL_DESCRIPTION_OBSERVER_H_
