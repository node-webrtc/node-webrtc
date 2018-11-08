/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/setsessiondescriptionobserver.h"

#include <webrtc/api/rtcerror.h>

#include "src/common.h"
#include "src/converters.h"
#include "src/converters/webrtc.h"
#include "src/error.h"  // IWYU pragma: keep
#include "src/errorfactory.h"  // IWYU pragma: keep
#include "src/functional/either.h"  // IWYU pragma: keep
#include "src/functional/validation.h"
#include "src/peerconnection.h"  // IWYU pragma: keep

// IWYU pragma: no_forward_declare node_webrtc::SomeError

using node_webrtc::Either;
using node_webrtc::ErrorFactory;
using node_webrtc::Errors;
using node_webrtc::From;
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

void SetSessionDescriptionObserver::OnFailure(const webrtc::RTCError error) {
  TRACE_CALL;
  if (_promise) {
    auto someError = From<SomeError>(&error).FromValidation([](Errors errors) {
      return SomeError(errors[0]);
    });
    // NOTE(mroberts): This workaround is annoying.
    if (someError.message().find("Local fingerprint does not match identity. Expected: ") != std::string::npos) {
      someError = SomeError(someError.message(),
              Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>::Left(
                  ErrorFactory::DOMExceptionName::kInvalidModificationError
              )
          );
    }
    _promise->Reject(someError);
    parent->Dispatch(std::move(_promise));
  }
  TRACE_END;
}
