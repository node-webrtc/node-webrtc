/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "src/converters/interfaces.h"

#include <nan.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>

#include "src/mediastream.h"  // IWYU pragma: keep
#include "src/mediastreamtrack.h"  // IWYU pragma: keep
#include "src/rtcrtpreceiver.h"  // IWYU pragma: keep
#include "src/rtcrtpsender.h"  // IWYU pragma: keep
#include "src/rtcrtptransceiver.h"  // IWYU pragma: keep

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
    auto isolate = Nan::GetCurrentContext()->GetIsolate(); \
    auto tpl = node_webrtc::IFACE::tpl().Get(isolate); \
    return tpl->HasInstance(value) \
        ? node_webrtc::Pure(FROM_FN(value->ToObject())) \
        : node_webrtc::Validation<node_webrtc::IFACE*>::Invalid("This is not an instance of " NAME); \
  }

#define CONVERT_INTERFACE_TO_AND_FROM_JS(IFACE, NAME, TO_FN, FROM_FN) \
  CONVERT_INTERFACE_TO_JS(IFACE, NAME, TO_FN) \
  CONVERT_INTERFACE_FROM_JS(IFACE, NAME, FROM_FN)

CONVERT_INTERFACE_TO_AND_FROM_JS(MediaStream, "MediaStream", handle, Nan::ObjectWrap::Unwrap<node_webrtc::MediaStream>)
CONVERT_INTERFACE_TO_AND_FROM_JS(MediaStreamTrack, "MediaStreamTrack", ToObject, node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::MediaStreamTrack>::Unwrap)
CONVERT_INTERFACE_TO_AND_FROM_JS(RTCRtpSender, "RTCRtpSender", ToObject, node_webrtc::AsyncObjectWrapWithLoop<node_webrtc::RTCRtpSender>::Unwrap)
CONVERT_INTERFACE_TO_JS(RTCRtpReceiver, "RTCRtpReceiver", ToObject)
CONVERT_INTERFACE_TO_JS(RTCRtpTransceiver, "RTCRtpTransceiver", ToObject)

CONVERTER_IMPL(node_webrtc::MediaStreamTrack*, rtc::scoped_refptr<webrtc::AudioTrackInterface>, mediaStreamTrack) {
  auto track = mediaStreamTrack->track();
  if (track->kind() != webrtc::MediaStreamTrackInterface::kAudioKind) {
    return node_webrtc::Validation<rtc::scoped_refptr<webrtc::AudioTrackInterface>>::Invalid(
            "Expected an audio MediaStreamTrack");
  }
  rtc::scoped_refptr<webrtc::AudioTrackInterface> audioTrack(static_cast<webrtc::AudioTrackInterface*>(track.get()));
  return node_webrtc::Pure(audioTrack);
}

CONVERTER_IMPL(node_webrtc::MediaStreamTrack*, rtc::scoped_refptr<webrtc::VideoTrackInterface>, mediaStreamTrack) {
  auto track = mediaStreamTrack->track();
  if (track->kind() != webrtc::MediaStreamTrackInterface::kVideoKind) {
    return node_webrtc::Validation<rtc::scoped_refptr<webrtc::VideoTrackInterface>>::Invalid(
            "Expected a video MediaStreamTrack");
  }
  rtc::scoped_refptr<webrtc::VideoTrackInterface> videoTrack(static_cast<webrtc::VideoTrackInterface*>(track.get()));
  return node_webrtc::Pure(videoTrack);
}

CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::MediaStreamTrack*, rtc::scoped_refptr<webrtc::AudioTrackInterface>)
CONVERT_VIA(v8::Local<v8::Value>, node_webrtc::MediaStreamTrack*, rtc::scoped_refptr<webrtc::VideoTrackInterface>)
