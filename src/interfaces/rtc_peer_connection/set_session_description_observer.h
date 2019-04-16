/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <webrtc/api/jsep.h>

#include "src/interfaces/rtc_peer_connection.h"
#include "src/node/promise.h"

namespace webrtc { class RTCError; }

namespace node_webrtc {

class SetSessionDescriptionObserver
  : public PromiseCreator<RTCPeerConnection>
  , public webrtc::SetSessionDescriptionObserver {
 public:
  SetSessionDescriptionObserver(
      RTCPeerConnection* peer_connection,
      Napi::Promise::Deferred deferred)
    : PromiseCreator<RTCPeerConnection>(peer_connection, deferred) {}

  void OnSuccess() override;

  void OnFailure(webrtc::RTCError) override;
};

}  // namespace node_webrtc
