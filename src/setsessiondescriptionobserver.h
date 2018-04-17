/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_SET_DESCRIPTION_DESCRIPTION_OBSERVER_H_
#define SRC_SET_DESCRIPTION_DESCRIPTION_OBSERVER_H_

#include <string>

#include "webrtc/api/jsep.h"

#include "src/converters/v8.h"
#include "src/events.h"

namespace node_webrtc {

class PeerConnection;

class SetSessionDescriptionObserver
  : public webrtc::SetSessionDescriptionObserver {
 private:
  PeerConnection* parent;
  std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection>> _promise;

 public:
  explicit SetSessionDescriptionObserver(
      PeerConnection* connection,
      std::unique_ptr<node_webrtc::PromiseEvent<PeerConnection>> promise)
    : parent(connection), _promise(std::move(promise)) {}

  virtual void OnSuccess();
  virtual void OnFailure(const std::string& msg);
};

}  // namespace node_webrtc

#endif  // SRC_SET_DESCRIPTION_DESCRIPTION_OBSERVER_H_
