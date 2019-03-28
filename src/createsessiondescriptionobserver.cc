/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/createsessiondescriptionobserver.h"

#include <type_traits>

#include <nan.h>
#include <webrtc/api/rtc_error.h>

#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/utility.h"

void node_webrtc::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* description) {
  auto validation = node_webrtc::From<RTCSessionDescriptionInit>(description);
  delete description;
  Dispatch([peer_connection = _peer_connection, validation](auto resolver) {
    if (validation.IsInvalid()) {
      node_webrtc::SomeError error(validation.ToErrors()[0]);
      node_webrtc::Reject(resolver, node_webrtc::SomeError(validation.ToErrors()[0]));
    } else {
      auto description = validation.UnsafeFromValid();
      peer_connection->SaveLastSdp(description);
      node_webrtc::Resolve(resolver, description);
    }
  });
}

void node_webrtc::CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error) {
  auto someError = node_webrtc::From<node_webrtc::SomeError>(&error).FromValidation([](auto errors) {
    return node_webrtc::SomeError(errors[0]);
  });
  Dispatch([someError](auto resolver) {
    node_webrtc::Reject(resolver, someError);
  });
}
