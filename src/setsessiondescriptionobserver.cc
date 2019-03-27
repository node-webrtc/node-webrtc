/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/setsessiondescriptionobserver.h"

#include <webrtc/api/rtc_error.h>

#include "src/converters.h"
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/errorfactory.h"
#include "src/functional/either.h"
#include "src/peerconnection.h"

node_webrtc::SetSessionDescriptionObserver::SetSessionDescriptionObserver(
    node_webrtc::PeerConnection* peer_connection)
  : _peer_connection(peer_connection) {
  Nan::HandleScope scope;
  _resolver = std::make_unique<Nan::Persistent<v8::Promise::Resolver>>(
          v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked());
}

node_webrtc::SetSessionDescriptionObserver::SetSessionDescriptionObserver(
    node_webrtc::PeerConnection* peer_connection,
    v8::Local<v8::Promise::Resolver> resolver)
  : _peer_connection(peer_connection) {
  Nan::HandleScope scope;
  _resolver = std::make_unique<Nan::Persistent<v8::Promise::Resolver>>(resolver);
}

void node_webrtc::SetSessionDescriptionObserver::OnSuccess() {
  _peer_connection->Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>(
  [_resolver = std::move(_resolver)]() {
    Nan::HandleScope scope;
    v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
    resolver->Resolve(Nan::GetCurrentContext(), Nan::Undefined()).IsNothing();
  }));
}

void node_webrtc::SetSessionDescriptionObserver::OnFailure(webrtc::RTCError error) {
  auto someError = node_webrtc::From<node_webrtc::SomeError>(&error).FromValidation([](node_webrtc::Errors errors) {
    return node_webrtc::SomeError(errors[0]);
  });

  // NOTE(mroberts): This workaround is annoying.
  if (someError.message().find("Local fingerprint does not match identity. Expected: ") != std::string::npos) {
    someError = node_webrtc::SomeError(someError.message(),
            node_webrtc::MakeLeft<node_webrtc::ErrorFactory::ErrorName>(
                node_webrtc::ErrorFactory::DOMExceptionName::kInvalidModificationError));
  }

  _peer_connection->Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>(
  [_resolver = std::move(_resolver), someError]() {
    Nan::HandleScope scope;
    v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
    CONVERT_OR_REJECT_AND_RETURN(resolver, someError, value, v8::Local<v8::Value>);
    resolver->Reject(Nan::GetCurrentContext(), value).IsNothing();
  }));
}

v8::Local<v8::Promise> node_webrtc::SetSessionDescriptionObserver::promise() {
  Nan::EscapableHandleScope scope;
  v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
  return scope.Escape(resolver->GetPromise());
}
