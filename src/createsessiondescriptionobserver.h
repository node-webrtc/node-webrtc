/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <webrtc/api/jsep.h>

#include "src/events.h"

namespace webrtc {

class RTCError;

}  // namespace webrtc

namespace node_webrtc {

struct RTCSessionDescriptionInit;
class PeerConnection;

class CreateSessionDescriptionObserver
  : public webrtc::CreateSessionDescriptionObserver {
 private:
  PeerConnection* parent;
  std::unique_ptr<PromiseEvent<PeerConnection, RTCSessionDescriptionInit>> _promise;

 public:
  CreateSessionDescriptionObserver(
      PeerConnection* parent,
      std::unique_ptr<PromiseEvent<PeerConnection, RTCSessionDescriptionInit>> promise)
    : parent(parent), _promise(std::move(promise)) {}

  void OnSuccess(webrtc::SessionDescriptionInterface* sdp) override;
  void OnFailure(webrtc::RTCError error) override;
};

}  // namespace node_webrtc
