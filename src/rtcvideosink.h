/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_RTCVIDEOSINK_H_
#define SRC_RTCVIDEOSINK_H_

#include <nan.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/asyncobjectwrapwithloop.h"  // IWYU pragma: keep

namespace node_webrtc { class OnFrameEvent; }
namespace webrtc { class VideoFrame; }
namespace webrtc { class VideoTrackInterface; }

namespace node_webrtc {

class RTCVideoSink
  : public AsyncObjectWrapWithLoop<RTCVideoSink>
  , public rtc::VideoSinkInterface<webrtc::VideoFrame> {
 public:
  ~RTCVideoSink() override = default;

  static void Init(v8::Handle<v8::Object> exports);

  virtual void OnFrame(const webrtc::VideoFrame& frame) override;

  void HandleOnFrameEvent(const OnFrameEvent& event);

 protected:
  void Stop() override;

 private:
  explicit RTCVideoSink(rtc::scoped_refptr<webrtc::VideoTrackInterface>);

  static Nan::Persistent<v8::FunctionTemplate>& tpl();

  static NAN_METHOD(New);

  static NAN_GETTER(GetStopped);

  static NAN_METHOD(JsStop);

  bool _stopped = false;
  rtc::scoped_refptr<webrtc::VideoTrackInterface> _track;
};

}  // namespace node_webrtc

#endif  // SRC_RTCVIDEOSINK_H_
