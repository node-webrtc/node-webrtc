#include "src/dictionaries/webrtc/video_frame_buffer.hh"

#include <libyuv.h>
#include <webrtc/api/video/i420_buffer.h>

#include "src/dictionaries/node_webrtc/image_data.hh"
#include "src/functional/validation.hh"

namespace node_webrtc {

static rtc::scoped_refptr<webrtc::I420Buffer>
CreateI420Buffer(I420ImageData i420Frame) {
  auto buffer =
      webrtc::I420Buffer::Create(i420Frame.width(), i420Frame.height());
  memcpy(buffer->MutableDataY(), i420Frame.dataY().data(),
         i420Frame.dataY().size_bytes());
  memcpy(buffer->MutableDataU(), i420Frame.dataU().data(),
         i420Frame.dataU().size_bytes());
  memcpy(buffer->MutableDataV(), i420Frame.dataV().data(),
         i420Frame.dataV().size_bytes());
  return buffer;
}

CONVERTER_IMPL(I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>, value) {
  return Pure(CreateI420Buffer(value));
}

TO_NAPI_IMPL(rtc::scoped_refptr<webrtc::VideoFrameBuffer>, pair) {
  auto value = pair.second;
  return value->type() == webrtc::VideoFrameBuffer::Type::kI420
             ? From<Napi::Value>(std::make_pair(pair.first, value->GetI420()))
             : Validation<Napi::Value>::Invalid(
                   "Unsupported RTCVideoFrame type (file a node-webrtc bug, "
                   "please!)");
}

CONVERT_VIA(Napi::Value, I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)

TO_NAPI_IMPL(const webrtc::I420BufferInterface *, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto value = pair.second;

  auto dstYStride = value->width();
  auto dstUVStride = (value->width() + 1) / 2;
  auto uvHeight = (value->height() + 1) / 2;

  auto sizeOfDstYPlane = dstYStride * value->height();
  auto sizeOfDstUPlane = dstUVStride * uvHeight;
  auto sizeOfDstVPlane = dstUVStride * uvHeight;

  auto byteLength = sizeOfDstYPlane + sizeOfDstUPlane + sizeOfDstVPlane;
  auto maybeArrayBuffer = Napi::ArrayBuffer::New(env, byteLength);
  if (maybeArrayBuffer.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(
        maybeArrayBuffer.Env().GetAndClearPendingException().Message());
  }
  auto data = static_cast<uint8_t *>(maybeArrayBuffer.Data());

  auto srcYPlane = value->DataY();
  auto srcUPlane = value->DataU();
  auto srcVPlane = value->DataV();

#pragma clang unsafe_buffer_usage begin
  auto dstYPlane = data;
  auto dstUPlane = data + sizeOfDstYPlane;
  auto dstVPlane = dstUPlane + sizeOfDstUPlane;
#pragma clang unsafe_buffer_usage end

  int rc = libyuv::I420Copy(
      srcYPlane, value->StrideY(), srcUPlane, value->StrideU(), srcVPlane,
      value->StrideV(), dstYPlane, dstYStride, dstUPlane, dstUVStride,
      dstVPlane, dstUVStride, value->width(), value->height());

  if (rc != 0) {
    return Validation<Napi::Value>::Invalid("Failed to copy I420 buffer: " +
                                            std::to_string(rc));
  }

  // FIXME(mroberts): How to create a Uint8ClampedArray?
  auto maybeUint8Array =
      Napi::Uint8Array::New(env, byteLength, maybeArrayBuffer, 0);
  if (maybeUint8Array.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(
        maybeUint8Array.Env().GetAndClearPendingException().Message());
  }

  return Pure(scope.Escape(maybeUint8Array));
}

} //  namespace node_webrtc
