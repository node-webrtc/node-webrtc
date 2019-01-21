/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_I420HELPERS_H_
#define SRC_I420HELPERS_H_

#include <nan.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/functional/validation.h"

namespace node_webrtc {

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

  Validation<I420ImageData> toI420() const;
  Validation<RgbaImageData> toRgba() const;
};

class I420ImageData {
 public:
  I420ImageData() = default;

  static node_webrtc::Validation<I420ImageData> Create(ImageData imageData);

  size_t sizeOfLuminancePlane() const {
    return static_cast<size_t>(width() * height());
  }

  size_t sizeOfChromaPlane() const {
    return sizeOfLuminancePlane() / 4;
  }

  uint8_t* dataY() const {
    return static_cast<uint8_t*>(data.contents.Data());
  }

  int strideY() const {
    return width();
  }

  uint8_t* dataU() const {
    return dataY() + sizeOfLuminancePlane();
  }

  int strideU() const {
    return width() / 2;
  }

  uint8_t* dataV() const {
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

  static node_webrtc::Validation<RgbaImageData> Create(ImageData imageData);

  uint8_t* dataRgba() const {
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

class I420Helpers {
 public:
  static void Init(v8::Handle<v8::Object> exports);

 private:
  static NAN_METHOD(I420ToRgba);
  static NAN_METHOD(RgbaToI420);
};

}  // namespace node_webrtc

#endif  // SRC_I420HELPERS_H_
