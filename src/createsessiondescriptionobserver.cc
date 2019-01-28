/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/createsessiondescriptionobserver.h"

#include <webrtc/api/jsep.h>
#include <webrtc/api/rtcerror.h>

#include "src/common.h"
#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/error.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/peerconnection.h"  // IWYU pragma: keep

// IWYU pragma: no_forward_declare node_webrtc::SomeError

using node_webrtc::CreateSessionDescriptionObserver;
using node_webrtc::Errors;
using node_webrtc::From;
using node_webrtc::PeerConnection;
using node_webrtc::SomeError;

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

void CreateSessionDescriptionObserver::OnFailure(const webrtc::RTCError error) {
  TRACE_CALL;
  if (_promise) {
    auto someError = From<SomeError>(&error).FromValidation([](Errors errors) {
      return SomeError(errors[0]);
    });
    _promise->Reject(someError);
    parent->Dispatch(std::move(_promise));
  }
  TRACE_END;
}
