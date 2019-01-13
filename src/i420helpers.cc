/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/i420helpers.h"

#include <libyuv/convert_argb.h>
#include <webrtc/api/video/i420_buffer.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/dictionaries.h"
#include "src/converters/v8.h"
#include "src/error.h"

NAN_METHOD(node_webrtc::I420Helpers::ARGB32ToI420) {
  Nan::ThrowError("Unimplemented!");
}

NAN_METHOD(node_webrtc::I420Helpers::I420ToARGB32) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<rtc::scoped_refptr<webrtc::I420Buffer> COMMA v8::ArrayBuffer::Contents>);

  rtc::scoped_refptr<webrtc::I420Buffer> i420Frame = std::get<0>(pair);
  v8::ArrayBuffer::Contents argb32Array = std::get<1>(pair);

  auto actualByteLength = argb32Array.ByteLength();
  auto expectedByteLength = i420Frame->width() * i420Frame->height() * 4;
  if (actualByteLength != expectedByteLength) {
    auto error = "ARGB32 should have .byteLength of " + std::to_string(expectedByteLength) + ", not " +
        std::to_string(actualByteLength);
    Nan::ThrowError(Nan::New(error).ToLocalChecked());
    return;
  }

  uint8_t* argb32Data = static_cast<uint8_t*>(argb32Array.Data());
  libyuv::I420ToARGB(
      i420Frame->DataY(),
      i420Frame->StrideY(),
      i420Frame->DataU(),
      i420Frame->StrideU(),
      i420Frame->DataV(),
      i420Frame->StrideV(),
      argb32Data,
      i420Frame->width() * 4,
      i420Frame->width(),
      i420Frame->height()
  );
}

void node_webrtc::I420Helpers::Init(v8::Handle<v8::Object> exports) {
  Nan::SetMethod(exports, "argb32ToI420", ARGB32ToI420);
  Nan::SetMethod(exports, "i420ToArgb32", I420ToARGB32);
}
