/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_CREATE_SESSION_DESCRIPTION_OBSERVER_H_
#define SRC_CREATE_SESSION_DESCRIPTION_OBSERVER_H_

#include <string>

#include "webrtc/api/jsep.h"

#include "src/converters/webrtc.h"
#include "src/events.h"

namespace node_webrtc {

class PeerConnection;

class CreateSessionDescriptionObserver
  : public webrtc::CreateSessionDescriptionObserver {
 private:
  PeerConnection* parent;
  std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection, node_webrtc::RTCSessionDescriptionInit>> _promise;

 public:
  explicit CreateSessionDescriptionObserver(
      PeerConnection* parent,
      std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection, node_webrtc::RTCSessionDescriptionInit>> promise)
    : parent(parent), _promise(std::move(promise)) {}

  virtual void OnSuccess(webrtc::SessionDescriptionInterface* sdp);
  virtual void OnFailure(const std::string& msg);
};

}  // namespace node_webrtc

#endif  // SRC_CREATE_SESSION_DESCRIPTION_OBSERVER_H_
