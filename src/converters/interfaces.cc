/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/interfaces.h"

#include "src/mediastream.h"
#include "src/mediastreamtrack.h"
#include "src/rtcrtpreceiver.h"
#include "src/rtcrtpsender.h"
#include "src/rtcrtptransceiver.h"

#define CONVERT_INTERFACE_TO_JS(IFACE, NAME, TO_FN) \
  TO_JS_IMPL(node_webrtc::IFACE*, value) { \
    Nan::EscapableHandleScope scope; \
    if (!value) { \
      return node_webrtc::Validation<v8::Local<v8::Value>>::Invalid(NAME " is null"); \
    } \
    return node_webrtc::Pure(scope.Escape(value->TO_FN()).As<v8::Value>()); \
  }

// FIXME(mroberts): This is not safe.
#define CONVERT_INTERFACE_FROM_JS(IFACE, NAME, FROM_FN) \
  FROM_JS_IMPL(node_webrtc::IFACE*, value) { \
    return value->IsObject() && !value->IsNull() && !value->IsArray() \
        ? node_webrtc::Pure(FROM_FN(value->ToObject())) \
        : node_webrtc::Validation<node_webrtc::IFACE*>::Invalid("This is not an instance of " NAME); \
  }

#define CONVERT_INTERFACE_TO_AND_FROM_JS(IFACE, NAME, TO_FN, FROM_FN) \
  CONVERT_INTERFACE_TO_JS(IFACE, NAME, TO_FN) \
  CONVERT_INTERFACE_FROM_JS(IFACE, NAME, FROM_FN)

CONVERT_INTERFACE_TO_AND_FROM_JS(MediaStream, "MediaStream", handle, Nan::ObjectWrap::Unwrap<node_webrtc::MediaStream>)
CONVERT_INTERFACE_TO_AND_FROM_JS(MediaStreamTrack, "MediaStreamTrack", ToObject, node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap)
CONVERT_INTERFACE_TO_AND_FROM_JS(RTCRtpSender, "RTCRtpSender", ToObject, node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCRtpSender>::Unwrap)
CONVERT_INTERFACE_TO_JS(RTCRtpReceiver, "RTCRtpReceiver", ToObject);
CONVERT_INTERFACE_TO_JS(RTCRtpTransceiver, "RTCRtpTransceiver", ToObject);
