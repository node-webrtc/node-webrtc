/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_peer_connection/create_session_description_observer.h"

#include <type_traits>

#include <nan.h>
#include <webrtc/api/rtc_error.h>

#include "src/converters/v8.h"
#include "src/dictionaries/node_webrtc/some_error.h"
#include "src/node/error.h"

void node_webrtc::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* description) {
  auto maybeDescription = node_webrtc::From<RTCSessionDescriptionInit>(description);
  delete description;
  if (maybeDescription.IsInvalid()) {
    Reject(node_webrtc::SomeError(maybeDescription.ToErrors()[0]));
  } else {
    auto description = maybeDescription.UnsafeFromValid();
    _peer_connection->SaveLastSdp(description);
    Resolve(description);
  }
}

void node_webrtc::CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error) {
  Reject(node_webrtc::From<node_webrtc::SomeError>(&error).FromValidation([](auto errors) {
    return node_webrtc::SomeError(errors[0]);
  }));
}
