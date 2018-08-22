/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "createsessiondescriptionobserver.h"

#include "common.h"
#include "peerconnection.h"

using node_webrtc::CreateSessionDescriptionObserver;
using node_webrtc::PeerConnection;

void CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp) {
  TRACE_CALL;
  if (_promise) {
    auto validation = From<RTCSessionDescriptionInit>(sdp);
    if (validation.IsInvalid()) {
      _promise->Reject(SomeError(validation.ToErrors()[0]));
    } else {
      auto description = validation.UnsafeFromValid();
      parent->SaveLastSdp(description);
      _promise->Resolve(description);
    }
    parent->Dispatch(std::move(_promise));
  }
  delete sdp;
  TRACE_END;
}

void CreateSessionDescriptionObserver::OnFailure(const std::string& error) {
  TRACE_CALL;
  if (_promise) {
    _promise->Reject(SomeError(error));
    parent->Dispatch(std::move(_promise));
  }
  TRACE_END;
}
