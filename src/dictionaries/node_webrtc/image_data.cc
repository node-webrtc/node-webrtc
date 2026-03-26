#include "src/dictionaries/node_webrtc/image_data.hh"

#include <node-addon-api/napi.h>
#include <webrtc/api/video/i420_buffer.h>

#include "src/converters.hh"
#include "src/converters/object.hh"
#include "src/dictionaries/webrtc/video_frame_buffer.hh"
#include "src/functional/curry.hh"
#include "src/functional/operators.hh"

namespace node_webrtc {

DECLARE_FROM_NAPI(ImageData)
FROM_NAPI_IMPL(ImageData, value) {
  return From<Napi::Object>(value).FlatMap<ImageData>([](auto object) {
    return curry(ImageData::Create) % GetRequired<int>(object, "width") *
           GetRequired<int>(object, "height") *
           GetRequired<Napi::ArrayBuffer>(object, "data");
  });
}

DECLARE_CONVERTER(ImageData, I420ImageData)
CONVERTER_IMPL(ImageData, I420ImageData, imageData) {
  return imageData.toI420();
}

CONVERT_VIA(Napi::Value, ImageData, I420ImageData)

DECLARE_CONVERTER(ImageData, RgbaImageData)
CONVERTER_IMPL(ImageData, RgbaImageData, imageData) {
  return imageData.toRgba();
}

CONVERT_VIA(Napi::Value, ImageData, RgbaImageData)

} // namespace node_webrtc
