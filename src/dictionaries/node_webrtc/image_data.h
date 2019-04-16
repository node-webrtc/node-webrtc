#pragma once

#include <node-addon-api/napi.h>

#include "src/converters/napi.h"
#include "src/functional/either.h"
#include "src/functional/validation.h"

namespace node_webrtc {

class I420ImageData;
class RgbaImageData;

class ImageData {
 public:
  int width;
  int height;
  Napi::ArrayBuffer contents;

  static ImageData Create(int width, int height, Napi::ArrayBuffer contents) {
    return {width, height, contents};
  }

  Validation<I420ImageData> toI420() const;
  Validation<RgbaImageData> toRgba() const;
};

class I420ImageData {
 public:
  I420ImageData() = default;

  static Validation<I420ImageData> Create(ImageData imageData);

  size_t sizeOfLuminancePlane() const {
    return static_cast<size_t>(width() * height());
  }

  size_t sizeOfChromaPlane() const {
    return sizeOfLuminancePlane() / 4;
  }

  uint8_t* dataY() {
    return static_cast<uint8_t*>(data.contents.Data());
  }

  int strideY() const {
    return width();
  }

  uint8_t* dataU() {
    return dataY() + sizeOfLuminancePlane();
  }

  int strideU() const {
    return width() / 2;
  }

  uint8_t* dataV() {
    return dataU() + sizeOfChromaPlane();
  }

  int strideV() const {
    return strideU();
  }

  int width() const {
    return data.width;
  }

  int height() const {
    return data.height;
  }

 private:
  explicit I420ImageData(const ImageData data): data(data) {}

  ImageData data;
};

class RgbaImageData {
 public:
  RgbaImageData() = default;

  static Validation<RgbaImageData> Create(ImageData imageData);

  uint8_t* dataRgba() {
    return static_cast<uint8_t*>(data.contents.Data());
  }

  int strideRgba() const {
    return width() * 4;
  }

  int width() const {
    return data.width;
  }

  int height() const {
    return data.height;
  }

 private:
  explicit RgbaImageData(const ImageData data): data(data) {}

  ImageData data;
};

DECLARE_FROM_NAPI(I420ImageData)
DECLARE_FROM_NAPI(RgbaImageData)

}  // namespace node_webrtc
