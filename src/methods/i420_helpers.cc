/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/methods/i420_helpers.h"

#include <libyuv.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/dictionaries/node_webrtc/image_data.h"
#include "src/node/error.h"

namespace node_webrtc {

Validation<I420ImageData> ImageData::toI420() const {
  return I420ImageData::Create(*this);
}

Validation<RgbaImageData> ImageData::toRgba() const {
  return RgbaImageData::Create(*this);
}

Validation<I420ImageData> I420ImageData::Create(const ImageData imageData) {
  auto expectedByteLength = static_cast<size_t>(imageData.width * imageData.height * 1.5);
  auto actualByteLength = imageData.contents.ByteLength();
  if (actualByteLength != expectedByteLength) {
    auto error = "Expected a .byteLength of " + std::to_string(expectedByteLength) + ", not " +
        std::to_string(actualByteLength);
    return Validation<I420ImageData>::Invalid(error);
  }
  I420ImageData i420ImageData(imageData);
  return Pure(i420ImageData);
}

Validation<RgbaImageData> RgbaImageData::Create(const ImageData imageData) {
  auto expectedByteLength = static_cast<size_t>(imageData.width * imageData.height * 4);
  auto actualByteLength = imageData.contents.ByteLength();
  if (actualByteLength != expectedByteLength) {
    auto error = "Expected a .byteLength of " + std::to_string(expectedByteLength) + ", not " +
        std::to_string(actualByteLength);
    return Validation<RgbaImageData>::Invalid(error);
  }
  RgbaImageData rgbaImageData(imageData);
  return Pure(rgbaImageData);
}

NAN_METHOD(I420Helpers::RgbaToI420) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<RgbaImageData COMMA I420ImageData>)

  RgbaImageData rgbaFrame = std::get<0>(pair);
  I420ImageData i420Frame = std::get<1>(pair);

  if (rgbaFrame.width() != i420Frame.width() || rgbaFrame.height() != i420Frame.height()) {
    Nan::ThrowError("Dimensions must match");
    return;
  }

  libyuv::ABGRToI420(
      rgbaFrame.dataRgba(),
      rgbaFrame.strideRgba(),
      i420Frame.dataY(),
      i420Frame.strideY(),
      i420Frame.dataU(),
      i420Frame.strideU(),
      i420Frame.dataV(),
      i420Frame.strideV(),
      rgbaFrame.width(),
      rgbaFrame.height()
  );
}

NAN_METHOD(I420Helpers::I420ToRgba) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<I420ImageData COMMA RgbaImageData>)

  I420ImageData i420Frame = std::get<0>(pair);
  RgbaImageData rgbaFrame = std::get<1>(pair);

  if (i420Frame.width() != rgbaFrame.width() || i420Frame.height() != rgbaFrame.height()) {
    Nan::ThrowError("Dimensions must match");
    return;
  }

  libyuv::I420ToABGR(
      i420Frame.dataY(),
      i420Frame.strideY(),
      i420Frame.dataU(),
      i420Frame.strideU(),
      i420Frame.dataV(),
      i420Frame.strideV(),
      rgbaFrame.dataRgba(),
      rgbaFrame.strideRgba(),
      i420Frame.width(),
      i420Frame.height()
  );
}

void I420Helpers::Init(v8::Handle<v8::Object> exports) {
  Nan::SetMethod(exports, "rgbaToI420", RgbaToI420);
  Nan::SetMethod(exports, "i420ToRgba", I420ToRgba);
}

}  // namespace node_webrtc
