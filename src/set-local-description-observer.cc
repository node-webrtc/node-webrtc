/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/set-local-description-observer.h"

#include "src/common.h"
#include "src/peerconnection.h"

using node_webrtc::PeerConnection;
using node_webrtc::SetLocalDescriptionObserver;

void SetLocalDescriptionObserver::OnSuccess() {
  TRACE_CALL;
  parent->Dispatch(SetLocalDescriptionSuccessEvent::Create());
  TRACE_END;
}

void SetLocalDescriptionObserver::OnFailure(const std::string& msg) {
  TRACE_CALL;
  parent->Dispatch(SetLocalDescriptionErrorEvent::Create(msg));
  TRACE_END;
}
