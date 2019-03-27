/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/createsessiondescriptionobserver.h"

#include <type_traits>

#include <webrtc/api/rtc_error.h>

#include "src/converters.h"
#include "src/converters/dictionaries.h"  // IWYU pragma: keep
#include "src/converters/v8.h"
#include "src/error.h"
#include "src/events.h"
#include "src/peerconnection.h"

node_webrtc::CreateSessionDescriptionObserver::CreateSessionDescriptionObserver(
    node_webrtc::PeerConnection* peer_connection)
  : _peer_connection(peer_connection) {
  Nan::HandleScope scope;
  _resolver = std::make_unique<Nan::Persistent<v8::Promise::Resolver>>(
          v8::Promise::Resolver::New(Nan::GetCurrentContext()).ToLocalChecked());
}

node_webrtc::CreateSessionDescriptionObserver::CreateSessionDescriptionObserver(
    node_webrtc::PeerConnection* peer_connection,
    v8::Local<v8::Promise::Resolver> resolver)
  : _peer_connection(peer_connection) {
  Nan::HandleScope scope;
  _resolver = std::make_unique<Nan::Persistent<v8::Promise::Resolver>>(resolver);
}

void node_webrtc::CreateSessionDescriptionObserver::OnSuccess(webrtc::SessionDescriptionInterface* description) {
  auto validation = node_webrtc::From<RTCSessionDescriptionInit>(description);
  delete description;
  _peer_connection->Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>(
  [peer_connection = _peer_connection, _resolver = std::move(_resolver), validation]() {
    Nan::HandleScope scope;
    v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);

    if (validation.IsInvalid()) {
      node_webrtc::SomeError error(validation.ToErrors()[0]);
      CONVERT_OR_REJECT_AND_RETURN(resolver, error, value, v8::Local<v8::Value>);
      resolver->Reject(Nan::GetCurrentContext(), value).IsNothing();
    } else {
      auto description = validation.UnsafeFromValid();
      peer_connection->SaveLastSdp(description);
      CONVERT_OR_REJECT_AND_RETURN(resolver, description, value, v8::Local<v8::Value>);
      resolver->Resolve(Nan::GetCurrentContext(), value).IsNothing();
    }
  }));
}

void node_webrtc::CreateSessionDescriptionObserver::OnFailure(webrtc::RTCError error) {
  auto someError = node_webrtc::From<node_webrtc::SomeError>(&error).FromValidation([](node_webrtc::Errors errors) {
    return node_webrtc::SomeError(errors[0]);
  });
  _peer_connection->Dispatch(node_webrtc::CreateCallback<node_webrtc::PeerConnection>(
  [_resolver = std::move(_resolver), someError]() {
    Nan::HandleScope scope;
    v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
    CONVERT_OR_REJECT_AND_RETURN(resolver, someError, value, v8::Local<v8::Value>);
    resolver->Reject(Nan::GetCurrentContext(), value).IsNothing();
  }));
}

v8::Local<v8::Promise> node_webrtc::CreateSessionDescriptionObserver::promise() {
  Nan::EscapableHandleScope scope;
  v8::Local<v8::Promise::Resolver> resolver = Nan::New(*_resolver);
  return scope.Escape(resolver->GetPromise());
}
