/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/createsessiondescriptionobserver.h"

#include <webrtc/api/jsep.h>
#include <webrtc/api/rtc_error.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/peerconnection.h"

void node_webrtc::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* sdp) {
  if (_promise) {
    auto validation = node_webrtc::From<RTCSessionDescriptionInit>(sdp);
    if (validation.IsInvalid()) {
      _promise->Reject(node_webrtc::SomeError(validation.ToErrors()[0]));
    } else {
      auto description = validation.UnsafeFromValid();
      parent->SaveLastSdp(description);
      _promise->Resolve(description);
    }
    parent->Dispatch(std::move(_promise));
  }
  delete sdp;
}

void node_webrtc::CreateSessionDescriptionObserver::OnFailure(const webrtc::RTCError error) {
  if (_promise) {
    auto someError = node_webrtc::From<node_webrtc::SomeError>(&error).FromValidation([](node_webrtc::Errors errors) {
      return node_webrtc::SomeError(errors[0]);
    });
    _promise->Reject(someError);
    parent->Dispatch(std::move(_promise));
  }
}
