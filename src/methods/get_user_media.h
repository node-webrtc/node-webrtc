/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <node-addon-api/napi.h>

// TODO Remove when `GetDisplayMedia` gets its own implementation
#include "src/methods/get_display_media.h"

namespace node_webrtc {

class GetUserMedia {
  // TODO Remove when `GetDisplayMedia` gets its own implementation
  friend class node_webrtc::GetDisplayMedia;
 public:
  static void Init(Napi::Env, Napi::Object);

 private:
  static Napi::Value GetUserMediaImpl(const Napi::CallbackInfo&);
};

}  // namespace node_webrtc
