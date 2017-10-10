/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/create-offer-observer.h"

#include "src/common.h"
#include "src/peerconnection.h"

using node_webrtc::CreateOfferObserver;
using node_webrtc::PeerConnection;
using webrtc::SessionDescriptionInterface;

void CreateOfferObserver::OnSuccess(SessionDescriptionInterface* sdp) {
  TRACE_CALL;
  _target->Dispatch(CreateOfferSuccessEvent::Create(
      std::move(_resolver), std::unique_ptr<SessionDescriptionInterface>(sdp)));
  TRACE_END;
}

void CreateOfferObserver::OnFailure(const std::string& msg) {
  TRACE_CALL;
  _target->Dispatch(CreateOfferErrorEvent::Create(std::move(_resolver), msg));
  TRACE_END;
}
