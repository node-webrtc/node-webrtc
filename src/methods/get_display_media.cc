/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/methods/get_display_media.h"

#include "src/methods/get_user_media.h"

void node_webrtc::GetDisplayMedia::Init(Napi::Env env, Napi::Object exports) {
  exports.Set("getDisplayMedia", Napi::Function::New(env, GetUserMedia::GetUserMediaImpl));
}
