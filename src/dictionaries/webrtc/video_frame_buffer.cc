#include "src/dictionaries/webrtc/video_frame_buffer.h"

#include <nan.h>
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

TO_JS_IMPL(rtc::scoped_refptr<webrtc::VideoFrameBuffer>, value) {
  return value->type() == webrtc::VideoFrameBuffer::Type::kI420
      ? node_webrtc::From<v8::Local<v8::Value>>(value->GetI420())
      : node_webrtc::Validation<v8::Local<v8::Value>>::Invalid("Unsupported RTCVideoFrame type (file a node-webrtc bug, please!)");
}

TO_JS_IMPL(rtc::scoped_refptr<webrtc::I420BufferInterface>, value) {
  Nan::EscapableHandleScope scope;
  auto isolate = Nan::GetCurrentContext()->GetIsolate();

  auto sizeOfYPlane = value->StrideY() * value->height();
  auto sizeOfUPlane = value->StrideU() * value->height() / 2;
  auto sizeOfVPlane = value->StrideV() * value->height() / 2;

  auto byteLength = sizeOfYPlane + sizeOfUPlane + sizeOfVPlane;
  auto arrayBuffer = v8::ArrayBuffer::New(isolate, byteLength);
  uint8_t* data = static_cast<uint8_t*>(arrayBuffer->GetContents().Data());

  auto srcYPlane = value->DataY();
  auto srcUPlane = value->DataU();
  auto srcVPlane = value->DataV();

  auto dstYPlane = data;
  auto dstUPlane = data + sizeOfYPlane;
  auto dstVPlane = dstUPlane + sizeOfUPlane;

  memcpy(dstYPlane, srcYPlane, sizeOfYPlane);
  memcpy(dstUPlane, srcUPlane, sizeOfUPlane);
  memcpy(dstVPlane, srcVPlane, sizeOfVPlane);

  auto uint8Array = v8::Uint8ClampedArray::New(arrayBuffer, 0, byteLength);
  return node_webrtc::Pure(scope.Escape(uint8Array.As<v8::Value>()));
}

CONVERT_VIA(v8::Local<v8::Value>, I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)

} //  namespace node_webrtc
