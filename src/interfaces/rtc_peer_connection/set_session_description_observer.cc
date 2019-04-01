/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/interfaces/rtc_peer_connection/set_session_description_observer.h"

#include <nan.h>
#include <webrtc/api/rtc_error.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/dictionaries/node_webrtc/some_error.h"
#include "src/functional/either.h"
#include "src/node/error_factory.h"

void node_webrtc::SetSessionDescriptionObserver::OnSuccess() {
  Resolve(node_webrtc::Undefined());
}

void node_webrtc::SetSessionDescriptionObserver::OnFailure(webrtc::RTCError error) {
  auto someError = node_webrtc::From<node_webrtc::SomeError>(&error).FromValidation([](auto errors) {
    return node_webrtc::SomeError(errors[0]);
  });

  // NOTE(mroberts): This workaround is annoying.
  if (someError.message().find("Local fingerprint does not match identity. Expected: ") != std::string::npos) {
    someError = node_webrtc::SomeError(someError.message(),
            node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(
                node_webrtc::ErrorFactory::DOMExceptionName::kInvalidModificationError));
  }

  Reject(someError);
}
