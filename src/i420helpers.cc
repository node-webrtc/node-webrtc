/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/i420helpers.h"

#include <libyuv.h>
#include <webrtc/api/video/i420_buffer.h>

#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/dictionaries.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"
#include "src/error.h"

class I420ImageData;
class RgbaImageData;

class ImageData {
 public:
  int width;
  int height;
  v8::ArrayBuffer::Contents contents;

  static ImageData Create(int width, int height, v8::ArrayBuffer::Contents contents) {
    return {width, height, contents};
  }

  node_webrtc::Validation<I420ImageData> toI420();
  node_webrtc::Validation<RgbaImageData> toRgba();
};

class I420ImageData {
 public:
  ImageData data;

  static node_webrtc::Validation<I420ImageData> Create(const ImageData imageData) {
    auto expectedByteLength = static_cast<size_t>(imageData.width * imageData.height * 1.5);
    auto actualByteLength = imageData.contents.ByteLength();
    if (actualByteLength != expectedByteLength) {
      auto error = "Expected a .byteLength of " + std::to_string(expectedByteLength) + ", not " +
          std::to_string(actualByteLength);
      return node_webrtc::Validation<I420ImageData>::Invalid(error);
    }
    I420ImageData i420ImageData = {imageData};
    return node_webrtc::Pure(i420ImageData);
  }
};

class RgbaImageData {
 public:
  ImageData data;

  static node_webrtc::Validation<RgbaImageData> Create(const ImageData imageData) {
    auto expectedByteLength = static_cast<size_t>(imageData.width * imageData.height * 4);
    auto actualByteLength = imageData.contents.ByteLength();
    if (actualByteLength != expectedByteLength) {
      auto error = "Expected a .byteLength of " + std::to_string(expectedByteLength) + ", not " +
          std::to_string(actualByteLength);
      return node_webrtc::Validation<RgbaImageData>::Invalid(error);
    }
    RgbaImageData rgbaImageData = {imageData};
    return node_webrtc::Pure(rgbaImageData);
  }
};

node_webrtc::Validation<I420ImageData> ImageData::toI420() {
  return I420ImageData::Create(*this);
}

node_webrtc::Validation<RgbaImageData> ImageData::toRgba() {
  return RgbaImageData::Create(*this);
}

namespace node_webrtc {

DECLARE_CONVERTER(v8::Local<v8::Value>, ImageData)
CONVERTER_IMPL(v8::Local<v8::Value>, ImageData, value) {
  return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<ImageData>(
  [](const v8::Local<v8::Object> object) {
    return curry(ImageData::Create)
        % node_webrtc::GetRequired<int>(object, "width")
        * node_webrtc::GetRequired<int>(object, "height")
        * node_webrtc::GetRequired<v8::ArrayBuffer::Contents>(object, "data");
  });
}

DECLARE_CONVERTER(ImageData, I420ImageData)
CONVERTER_IMPL(ImageData, I420ImageData, imageData) {
  return imageData.toI420();
}

DECLARE_CONVERTER(v8::Local<v8::Value>, I420ImageData)
CONVERT_VIA(v8::Local<v8::Value>, ImageData, I420ImageData)

DECLARE_CONVERTER(ImageData, RgbaImageData)
CONVERTER_IMPL(ImageData, RgbaImageData, imageData) {
  return imageData.toRgba();
}

DECLARE_CONVERTER(v8::Local<v8::Value>, RgbaImageData)
CONVERT_VIA(v8::Local<v8::Value>, ImageData, RgbaImageData)

}  // namespace node_webrtc

NAN_METHOD(node_webrtc::I420Helpers::RgbaToI420) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<RgbaImageData COMMA I420ImageData>);

  RgbaImageData rgbaFrame = std::get<0>(pair);
  I420ImageData i420Frame = std::get<1>(pair);

  if (i420Frame.data.width != rgbaFrame.data.width || i420Frame.data.height != rgbaFrame.data.height) {
    Nan::ThrowError("Dimensions must match");
    return;
  }

  auto width = i420Frame.data.width;
  auto height = i420Frame.data.height;
  auto sizeOfLuminancePlane = width * height;
  auto sizeOfChromaPlane = sizeOfLuminancePlane / 4;

  auto dataY = static_cast<uint8_t*>(i420Frame.data.contents.Data());
  auto strideY = width;
  auto dataU = dataY + sizeOfLuminancePlane;
  auto strideU = strideY / 2;
  auto dataV = dataU + sizeOfChromaPlane;
  auto strideV = strideU;

  auto dataRgba = static_cast<uint8_t*>(rgbaFrame.data.contents.Data());
  auto strideRgba = width * 4;

  libyuv::ABGRToI420(
      dataRgba,
      strideRgba,
      dataY,
      strideY,
      dataU,
      strideU,
      dataV,
      strideV,
      width,
      height
  );
}

NAN_METHOD(node_webrtc::I420Helpers::I420ToRgba) {
  CONVERT_ARGS_OR_THROW_AND_RETURN(pair, std::tuple<I420ImageData COMMA RgbaImageData>);

  I420ImageData i420Frame = std::get<0>(pair);
  RgbaImageData rgbaFrame = std::get<1>(pair);

  if (i420Frame.data.width != rgbaFrame.data.width || i420Frame.data.height != rgbaFrame.data.height) {
    Nan::ThrowError("Dimensions must match");
    return;
  }

  auto width = i420Frame.data.width;
  auto height = i420Frame.data.height;
  auto sizeOfLuminancePlane = width * height;
  auto sizeOfChromaPlane = sizeOfLuminancePlane / 4;

  auto dataY = static_cast<uint8_t*>(i420Frame.data.contents.Data());
  auto strideY = width;
  auto dataU = dataY + sizeOfLuminancePlane;
  auto strideU = strideY / 2;
  auto dataV = dataU + sizeOfChromaPlane;
  auto strideV = strideU;

  auto dataRgba = static_cast<uint8_t*>(rgbaFrame.data.contents.Data());
  auto strideRgba = width * 4;

  libyuv::I420ToABGR(
      dataY,
      strideY,
      dataU,
      strideU,
      dataV,
      strideV,
      dataRgba,
      strideRgba,
      width,
      height
  );
}

void node_webrtc::I420Helpers::Init(v8::Handle<v8::Object> exports) {
  Nan::SetMethod(exports, "rgbaToI420", RgbaToI420);
  Nan::SetMethod(exports, "i420ToRgba", I420ToRgba);
}
