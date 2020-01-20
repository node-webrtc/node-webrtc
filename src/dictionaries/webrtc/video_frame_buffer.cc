#include "src/dictionaries/webrtc/video_frame_buffer.h"

#include <webrtc/api/video/i420_buffer.h>

#include "src/dictionaries/node_webrtc/image_data.h"
#include "src/functional/validation.h"

namespace node_webrtc {

static rtc::scoped_refptr<webrtc::I420Buffer> CreateI420Buffer(
    I420ImageData i420Frame) {
  auto buffer = webrtc::I420Buffer::Create(i420Frame.width(), i420Frame.height());
  memcpy(buffer->MutableDataY(), i420Frame.dataY(), i420Frame.sizeOfLuminancePlane());
  memcpy(buffer->MutableDataU(), i420Frame.dataU(), i420Frame.sizeOfChromaPlane());
  memcpy(buffer->MutableDataV(), i420Frame.dataV(), i420Frame.sizeOfChromaPlane());
  return buffer;
}

CONVERTER_IMPL(I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>, value) {
  return Pure(CreateI420Buffer(value));
}

TO_NAPI_IMPL(rtc::scoped_refptr<webrtc::VideoFrameBuffer>, pair) {
  auto value = pair.second;
  return value->type() == webrtc::VideoFrameBuffer::Type::kI420
      ? From<Napi::Value>(std::make_pair(pair.first, value->GetI420()))
      : Validation<Napi::Value>::Invalid("Unsupported RTCVideoFrame type (file a node-webrtc bug, please!)");
}

CONVERT_VIA(Napi::Value, I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)

TO_NAPI_IMPL(const webrtc::I420BufferInterface*, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto value = pair.second;

  auto sizeOfSrcYPlane = value->StrideY() * value->height();
  auto sizeOfSrcUPlane = value->StrideU() * value->height() / 2;
  auto sizeOfSrcVPlane = value->StrideV() * value->height() / 2;
  auto sizeOfDstYPlane = value->width() * value->height();
  auto sizeOfDstUPlane = sizeOfDstYPlane / 4;
  auto sizeOfDstVPlane = sizeOfDstYPlane / 4;

  auto byteLength = sizeOfDstYPlane + sizeOfDstUPlane + sizeOfDstVPlane;
  auto maybeArrayBuffer = Napi::ArrayBuffer::New(env, byteLength);
  if (maybeArrayBuffer.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(maybeArrayBuffer.Env().GetAndClearPendingException().Message());
  }
  auto data = static_cast<uint8_t*>(maybeArrayBuffer.Data());

  auto srcYPlane = value->DataY();
  auto srcUPlane = value->DataU();
  auto srcVPlane = value->DataV();

  auto dstYPlane = data;
  auto dstUPlane = data + sizeOfDstYPlane;
  auto dstVPlane = dstUPlane + sizeOfDstUPlane;

  if (sizeOfSrcYPlane == sizeOfDstYPlane) {
    memcpy(dstYPlane, srcYPlane, sizeOfDstYPlane);
  } else {
    for (int i = 0, j = 0; i < sizeOfSrcYPlane; i += value->StrideY(), j += value->width()) {
      memcpy(dstYPlane + j, srcYPlane + i, value->width());
    }
  }
  if (sizeOfSrcUPlane == sizeOfDstUPlane) {
    memcpy(dstUPlane, srcUPlane, sizeOfDstUPlane);
  } else {
    for (int i = 0, j = 0; i < sizeOfSrcUPlane; i += value->StrideU(), j += value->width() / 2) {
      memcpy(dstUPlane + j, srcUPlane + i, value->width() / 2);
    }
  }
  if (sizeOfSrcVPlane == sizeOfDstVPlane) {
    memcpy(dstVPlane, srcVPlane, sizeOfDstVPlane);
  } else {
    for (int i = 0, j = 0; i < sizeOfSrcVPlane; i += value->StrideV(), j += value->width() / 2) {
      memcpy(dstVPlane + j, srcVPlane + i, value->width() / 2);
    }
  }

  // FIXME(mroberts): How to create a Uint8ClampedArray?
  auto maybeUint8Array = Napi::Uint8Array::New(env, byteLength, maybeArrayBuffer, 0);
  if (maybeUint8Array.Env().IsExceptionPending()) {
    return Validation<Napi::Value>::Invalid(maybeUint8Array.Env().GetAndClearPendingException().Message());
  }

  return Pure(scope.Escape(maybeUint8Array));
}

} //  namespace node_webrtc
