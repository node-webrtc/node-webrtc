/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "setsessiondescriptionobserver.h"

#include "common.h"
#include "peerconnection.h"

using node_webrtc::PeerConnection;
using node_webrtc::SetSessionDescriptionObserver;
using node_webrtc::SomeError;

void SetSessionDescriptionObserver::OnSuccess() {
  TRACE_CALL;
  if (_promise) {
    _promise->Resolve(Undefined());
    parent->Dispatch(std::move(_promise));
  }
  TRACE_END;
}

void SetSessionDescriptionObserver::OnFailure(const std::string& error) {
  TRACE_CALL;
  if (_promise) {
    _promise->Reject(SomeError(error));
    parent->Dispatch(std::move(_promise));
  }
  TRACE_END;
}
