/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/dictionaries.h"

#include <nan.h>
#include <webrtc/api/data_channel_interface.h>
#include <webrtc/api/jsep.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/rtc_error.h>
#include <webrtc/api/rtp_parameters.h>
#include <webrtc/api/rtp_receiver_interface.h>
#include <webrtc/api/rtp_transceiver_interface.h>
#include <webrtc/api/stats/rtc_stats.h>
#include <webrtc/api/video/i420_buffer.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/interfaces.h"
#include "src/converters/object.h"
#include "src/converters/v8.h"
#include "src/functional/curry.h"
#include "src/functional/maybe.h"
#include "src/functional/operators.h"
#include "src/functional/either.h"
#include "src/i420helpers.h"
#include "src/errorfactory.h"
#include "src/interfaces/mediastream.h"
#include "src/interfaces/mediastreamtrack.h"
#include "src/interfaces/rtcrtpreceiver.h"
#include "src/interfaces/rtcrtpsender.h"
#include "src/interfaces/rtcrtptransceiver.h"
#include "src/interfaces/rtcstatsresponse.h"

#include "src/dictionaries/webrtc/data_channel_init.h"
#include "src/dictionaries/webrtc/ice_candidate_interface.h"
#include "src/dictionaries/webrtc/ice_server.h"
#include "src/dictionaries/webrtc/rtp_transceiver_init.h"




TO_JS_IMPL(webrtc::RtpSource, source) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("timestamp").ToLocalChecked(), Nan::New<v8::Number>(source.timestamp_ms()));
  object->Set(Nan::New("source").ToLocalChecked(), Nan::New(source.source_id()));
  return node_webrtc::Pure(scope.Escape(object).As<v8::Value>());
}

TO_JS_IMPL(webrtc::RtpHeaderExtensionParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("uri").ToLocalChecked(), Nan::New(params.uri).ToLocalChecked());
  object->Set(Nan::New("id").ToLocalChecked(), Nan::New(params.id));
  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

TO_JS_IMPL(webrtc::RtpCodecParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("payloadType").ToLocalChecked(), Nan::New(params.payload_type));
  object->Set(Nan::New("mimeType").ToLocalChecked(), Nan::New(params.mime_type()).ToLocalChecked());
  if (params.clock_rate) {
    object->Set(Nan::New("clockRate").ToLocalChecked(), Nan::New(*params.clock_rate));
  }
  if (params.num_channels) {
    object->Set(Nan::New("channels").ToLocalChecked(), Nan::New(*params.num_channels));
  }
  if (!params.parameters.empty()) {
    std::string fmtp("a=fmtp:" + std::to_string(params.payload_type));
    unsigned long i = 0;
    for (auto param : params.parameters) {
      fmtp += " " + param.first + "=" + param.second;
      if (i < params.parameters.size() - 1) {
        fmtp += ";";
      }
      i++;
    }
    object->Set(Nan::New("sdpFmtpLine").ToLocalChecked(), Nan::New(fmtp).ToLocalChecked());
  }
  return node_webrtc::Pure(scope.Escape(object).As<v8::Value>());
}

TO_JS_IMPL(webrtc::RtcpParameters, params) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  if (!params.cname.empty()) {
    object->Set(Nan::New("cname").ToLocalChecked(), Nan::New(params.cname).ToLocalChecked());
  }
  object->Set(Nan::New("reducedSize").ToLocalChecked(), Nan::New(params.reduced_size));
  return node_webrtc::Pure(scope.Escape(object.As<v8::Value>()));
}

static v8::Local<v8::Value> CreateRtpParameters(v8::Local<v8::Value> headerExtensions, v8::Local<v8::Value> codecs, v8::Local<v8::Value> rtcp) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  object->Set(Nan::New("headerExtensions").ToLocalChecked(), headerExtensions);
  object->Set(Nan::New("codecs").ToLocalChecked(), codecs);
  object->Set(Nan::New("encodings").ToLocalChecked(), Nan::New<v8::Array>());
  object->Set(Nan::New("rtcp").ToLocalChecked(), rtcp);
  return scope.Escape(object);
}

TO_JS_IMPL(webrtc::RtpParameters, params) {
  return curry(CreateRtpParameters)
      % node_webrtc::From<v8::Local<v8::Value>>(params.header_extensions)
      * node_webrtc::From<v8::Local<v8::Value>>(params.codecs)
      * node_webrtc::From<v8::Local<v8::Value>>(params.rtcp);
}

namespace node_webrtc {

DECLARE_CONVERTER(v8::Local<v8::Value>, node_webrtc::ImageData)
CONVERTER_IMPL(v8::Local<v8::Value>, node_webrtc::ImageData, value) {
  return node_webrtc::From<v8::Local<v8::Object>>(value).FlatMap<node_webrtc::ImageData>(
  [](const v8::Local<v8::Object> object) {
    return curry(node_webrtc::ImageData::Create)
        % node_webrtc::GetRequired<int>(object, "width")
        * node_webrtc::GetRequired<int>(object, "height")
        * node_webrtc::GetRequired<v8::ArrayBuffer::Contents>(object, "data");
  });
}

DECLARE_CONVERTER(node_webrtc::ImageData, I420ImageData)
CONVERTER_IMPL(node_webrtc::ImageData, I420ImageData, imageData) {
  return imageData.toI420();
}

CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::ImageData, I420ImageData)

DECLARE_CONVERTER(node_webrtc::ImageData, RgbaImageData)
CONVERTER_IMPL(node_webrtc::ImageData, RgbaImageData, imageData) {
  return imageData.toRgba();
}

CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::ImageData, RgbaImageData)

DECLARE_CONVERTER(node_webrtc::I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)
CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>)

}  // namespace node_webrtc

static rtc::scoped_refptr<webrtc::I420Buffer> CreateI420Buffer(
    node_webrtc::I420ImageData i420Frame
) {
  auto buffer = webrtc::I420Buffer::Create(i420Frame.width(), i420Frame.height());
  memcpy(buffer->MutableDataY(), i420Frame.dataY(), i420Frame.sizeOfLuminancePlane());
  memcpy(buffer->MutableDataU(), i420Frame.dataU(), i420Frame.sizeOfChromaPlane());
  memcpy(buffer->MutableDataV(), i420Frame.dataV(), i420Frame.sizeOfChromaPlane());
  return buffer;
}

CONVERTER_IMPL(node_webrtc::I420ImageData, rtc::scoped_refptr<webrtc::I420Buffer>, value) {
  return node_webrtc::Pure(CreateI420Buffer(value));
}

namespace node_webrtc {

}  // namespace node_webrtc

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

TO_JS_IMPL(webrtc::VideoFrame, value) {
  Nan::EscapableHandleScope scope;
  auto frame = Nan::New<v8::Object>();
  frame->Set(Nan::New("width").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(value.width()).UnsafeFromValid());
  frame->Set(Nan::New("height").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(value.height()).UnsafeFromValid());
  frame->Set(Nan::New("rotation").ToLocalChecked(), node_webrtc::From<v8::Local<v8::Value>>(static_cast<int>(value.rotation())).UnsafeFromValid());
  auto maybeData = node_webrtc::From<v8::Local<v8::Value>>(value.video_frame_buffer());
  if (maybeData.IsInvalid()) {
    return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid(maybeData.ToErrors()[0]);
  }
  auto data = maybeData.UnsafeFromValid();
  frame->Set(Nan::New("data").ToLocalChecked(), data);
  return node_webrtc::Pure(scope.Escape(frame).As<v8::Value>());
}
