/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_SET_REMOTE_DESCRIPTION_OBSERVER_H_
#define SRC_SET_REMOTE_DESCRIPTION_OBSERVER_H_

#include <string>

#include <nan.h>
#include <v8.h>

#include "webrtc/api/jsep.h"

namespace node_webrtc {

class PeerConnection;

class SetRemoteDescriptionObserver
: public webrtc::SetSessionDescriptionObserver {
 public:
  SetRemoteDescriptionObserver(
    PeerConnection* target
  , v8::Local<v8::Promise::Resolver> resolver)
  : _target(target)
  , _resolver(std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>>(
    new Nan::Persistent<v8::Promise::Resolver>(resolver))) {}

  void OnSuccess() override;
  void OnFailure(const std::string& msg) override;

 private:
  std::unique_ptr<Nan::Persistent<v8::Promise::Resolver>> _resolver;
  PeerConnection* _target;
};

}  // namespace node_webrtc

#endif  // SRC_SET_REMOTE_DESCRIPTION_OBSERVER_H_
