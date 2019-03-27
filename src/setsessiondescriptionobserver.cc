/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/setsessiondescriptionobserver.h"

#include <webrtc/api/rtcerror.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/errorfactory.h"
#include "src/functional/either.h"
#include "src/peerconnection.h"

void node_webrtc::SetSessionDescriptionObserver::OnSuccess() {
  if (_promise) {
    _promise->Resolve(Undefined());
    parent->Dispatch(std::move(_promise));
  }
}

void node_webrtc::SetSessionDescriptionObserver::OnFailure(const webrtc::RTCError error) {
  if (_promise) {
    auto someError = node_webrtc::From<node_webrtc::SomeError>(&error).FromValidation([](node_webrtc::Errors errors) {
      return node_webrtc::SomeError(errors[0]);
    });
    // NOTE(mroberts): This workaround is annoying.
    if (someError.message().find("Local fingerprint does not match identity. Expected: ") != std::string::npos) {
      someError = node_webrtc::SomeError(someError.message(),
              node_webrtc::Either<node_webrtc::ErrorFactory::DOMExceptionName, node_webrtc::ErrorFactory::ErrorName>::Left(
                  node_webrtc::ErrorFactory::DOMExceptionName::kInvalidModificationError
              )
          );
    }
    _promise->Reject(someError);
    parent->Dispatch(std::move(_promise));
  }
}
