/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCVIDEOSOURCE_H_
#define SRC_RTCVIDEOSOURCE_H_

#include <nan.h>
#include <v8.h>  // IWYU pragma: keep

#include <webrtc/api/mediastreaminterface.h>

// TODO(mroberts): Change to VideoTrackSource.

namespace node_webrtc {

class RTCVideoSource
  : public Nan::ObjectWrap {
 public:
  RTCVideoSource() = delete;

  ~RTCVideoSource() override = default;

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);

 private:
  explicit RTCVideoSource(rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> source)
    : _source(source) {}

  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static NAN_GETTER(GetIsScreencast);
  static NAN_GETTER(GetNeedsDenoising);
  static NAN_GETTER(GetState);
  static NAN_GETTER(GetRemote);

  static NAN_METHOD(GetStats);

  rtc::scoped_refptr<webrtc::VideoTrackSourceInterface> _source;
};

}  // namespace node_webrtc

#endif  // SRC_RTCVIDEOSOURCE_H_
