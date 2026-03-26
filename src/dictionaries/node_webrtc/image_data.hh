#pragma once

#include <node-addon-api/napi.h>
#include <span>

#include "src/converters/napi.hh"
#include "src/functional/validation.hh"

namespace node_webrtc {

class I420ImageData;
class RgbaImageData;

class ImageData {
public:
  size_t width;
  size_t height;
  Napi::ArrayBuffer contents;

  static ImageData Create(size_t width, size_t height,
                          Napi::ArrayBuffer contents) {
    return {width, height, contents};
  }

  [[nodiscard]] Validation<I420ImageData> toI420() const;
  [[nodiscard]] Validation<RgbaImageData> toRgba() const;
};

class I420ImageData {
public:
  I420ImageData() = default;

  static Validation<I420ImageData> Create(ImageData imageData);

  [[nodiscard]] size_t sizeOfLuminancePlane() const {
    return static_cast<size_t>(width()) * height();
  }

  [[nodiscard]] size_t sizeOfChromaPlane() const {
    return static_cast<size_t>((width() + 1) / 2) *
           static_cast<size_t>((height() + 1) / 2);
  }

  std::span<uint8_t> dataY() {
    auto ptr = static_cast<uint8_t *>(data.contents.Data());
    auto len = sizeOfLuminancePlane();
    return {ptr, len};
  }

  [[nodiscard]] size_t strideY() const { return width(); }

  std::span<uint8_t> dataU() {
#pragma clang unsafe_buffer_usage begin
    auto ptr =
        static_cast<uint8_t *>(data.contents.Data()) + sizeOfLuminancePlane();
    auto len = sizeOfChromaPlane();
#pragma clang unsafe_buffer_usage end
    return {ptr, len};
  }

  [[nodiscard]] size_t strideU() const { return (width() + 1) / 2; }

  std::span<uint8_t> dataV() {
#pragma clang unsafe_buffer_usage begin
    auto ptr = static_cast<uint8_t *>(data.contents.Data()) +
               sizeOfLuminancePlane() + sizeOfChromaPlane();
    auto len = sizeOfChromaPlane();
#pragma clang unsafe_buffer_usage end
    return {ptr, len};
  }

  [[nodiscard]] size_t strideV() const { return strideU(); }

  [[nodiscard]] size_t width() const { return data.width; }

  [[nodiscard]] size_t height() const { return data.height; }

private:
  explicit I420ImageData(const ImageData data) : data(data) {}

  ImageData data;
};

class RgbaImageData {
public:
  RgbaImageData() = default;

  static Validation<RgbaImageData> Create(ImageData imageData);

  uint8_t *dataRgba() { return static_cast<uint8_t *>(data.contents.Data()); }

  [[nodiscard]] size_t strideRgba() const { return width() * 4; }

  [[nodiscard]] size_t width() const { return data.width; }

  [[nodiscard]] size_t height() const { return data.height; }

private:
  explicit RgbaImageData(const ImageData data) : data(data) {}

  ImageData data;
};

DECLARE_FROM_NAPI(I420ImageData)
DECLARE_FROM_NAPI(RgbaImageData)

} // namespace node_webrtc
