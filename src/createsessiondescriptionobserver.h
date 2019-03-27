/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>

#include <nan.h>
#include <webrtc/api/jsep.h>
#include <v8.h>

namespace webrtc { class RTCError; }

namespace node_webrtc {

class PeerConnection;

class CreateSessionDescriptionObserver
  : public webrtc::CreateSessionDescriptionObserver {
 public:
  explicit CreateSessionDescriptionObserver(PeerConnection*);

  CreateSessionDescriptionObserver(PeerConnection*, v8::Local<v8::Promise::Resolver>);

  void OnSuccess(webrtc::SessionDescriptionInterface*) override;

  void OnFailure(webrtc::RTCError) override;

  v8::Local<v8::Promise> promise();

 private:
  PeerConnection* _peer_connection;
  std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
};

}  // namespace node_webrtc
