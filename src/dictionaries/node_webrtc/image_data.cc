#include "src/dictionaries/node_webrtc/image_data.h"

#include <node-addon-api/napi.h>
#include <v8.h>
#include <webrtc/api/video/i420_buffer.h>

#include "src/converters.h"
#include "src/converters/object.h"
#include "src/dictionaries/webrtc/video_frame_buffer.h"
#include "src/functional/curry.h"
#include "src/functional/operators.h"

namespace node_webrtc {

DECLARE_CONVERTER(v8::Local<v8::Value>, ImageData)
CONVERTER_IMPL(v8::Local<v8::Value>, ImageData, value) {
  return From<v8::Local<v8::Object>>(value).FlatMap<ImageData>([](auto object) {
    return curry(ImageData::Create)
        % GetRequired<int>(object, "width")
        * GetRequired<int>(object, "height")
        * GetRequired<v8::ArrayBuffer::Contents>(object, "data");
  });
}

DECLARE_FROM_NAPI(ImageData)
FROM_NAPI_IMPL(ImageData, value) {
  return From<Napi::Object>(value).FlatMap<ImageData>([](auto object) {
    return curry(ImageData::Create)
        % napi::GetRequired<int>(object, "width")
        * napi::GetRequired<int>(object, "height")
        * Validation<v8::ArrayBuffer::Contents>::Invalid("// FIXME(mroberts): Unsupported");
  });
}

DECLARE_CONVERTER(ImageData, I420ImageData)
CONVERTER_IMPL(ImageData, I420ImageData, imageData) {
  return imageData.toI420();
}

CONVERT_VIA(v8::Local<v8::Value>, ImageData, I420ImageData)
CONVERT_VIA(Napi::Value, ImageData, I420ImageData)

DECLARE_CONVERTER(ImageData, RgbaImageData)
CONVERTER_IMPL(ImageData, RgbaImageData, imageData) {
  return imageData.toRgba();
}

CONVERT_VIA(v8::Local<v8::Value>, ImageData, RgbaImageData)
CONVERT_VIA(Napi::Value, ImageData, RgbaImageData)

}  // namespace node_webrtc
