/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */

/*
 * This file defines conversion functions between WebRTC and v8 data types. We
 * try to match the W3C-specced Web IDL as closely as possible.
 */

#pragma once

#include <cstdint>
#include <iosfwd>
#include <map>
#include <utility>
#include <vector>

#include <absl/types/optional.h>
#include <nan.h>
#include <webrtc/api/media_stream_interface.h>
#include <webrtc/api/peer_connection_interface.h>
#include <v8.h>

#include "src/converters.h"
#include "src/converters/enums.h"
#include "src/functional/maybe.h"
#include "src/functional/validation.h"

#include "src/dictionaries/node_webrtc/rtc_answer_options.h"
#include "src/dictionaries/node_webrtc/rtc_dtls_fingerprint.h"
#include "src/dictionaries/node_webrtc/rtc_offer_options.h"
#include "src/dictionaries/node_webrtc/rtc_on_data_event_dict.h"
#include "src/dictionaries/node_webrtc/rtc_outh_credential.h"
#include "src/dictionaries/node_webrtc/rtc_session_description_init.h"
#include "src/dictionaries/node_webrtc/rtc_video_source_init.h"
#include "src/dictionaries/node_webrtc/unsigned_short_range.h"

#include "src/dictionaries/webrtc/ice_candidate_interface.h"
#include "src/dictionaries/webrtc/data_channel_init.h"
#include "src/dictionaries/webrtc/rtp_transceiver_init.h"

namespace rtc {

template <class T> class scoped_refptr;

}  // namespace rtc

namespace webrtc {

class I420Buffer;
class I420BufferInterface;
class RTCError;
class RTCStats;
class RTCStatsMemberInterface;
class RTCStatsReport;
struct RtcpParameters;
struct RtpCodecParameters;
struct RtpExtension;
struct RtpParameters;
class RtpSource;
class SessionDescriptionInterface;
class VideoFrame;
class VideoFrameBuffer;

}  // namespace webrtc;

namespace node_webrtc {

class I420ImageData;
class MediaStream;
class RgbaImageData;
class SomeError;

struct ExtendedRTCConfiguration {
  ExtendedRTCConfiguration(): configuration(webrtc::PeerConnectionInterface::RTCConfiguration()), portRange(UnsignedShortRange()) {}
  ExtendedRTCConfiguration(const webrtc::PeerConnectionInterface::RTCConfiguration& configuration, const UnsignedShortRange portRange): configuration(configuration), portRange(portRange) {}
  webrtc::PeerConnectionInterface::RTCConfiguration configuration;
  UnsignedShortRange portRange;
};

// TODO(mroberts): Move this elsewhere.
template <typename T>
struct Converter<absl::optional<T>, v8::Local<v8::Value>> {
  static Validation<v8::Local<v8::Value>> Convert(absl::optional<T> value) {
    if (value) {
      return Converter<T, v8::Local<v8::Value>>::Convert(*value);
    }
    return Pure(Nan::Null().As<v8::Value>());
  }
};

DECLARE_TO_JS(rtc::scoped_refptr<webrtc::RTCStatsReport>)
DECLARE_CONVERTER(webrtc::RTCError*, SomeError)
DECLARE_CONVERTER(webrtc::PeerConnectionInterface::IceConnectionState, RTCPeerConnectionState)
DECLARE_FROM_JS(webrtc::PeerConnectionInterface::RTCConfiguration)
DECLARE_TO_JS(webrtc::RtpCodecParameters)
DECLARE_TO_JS(webrtc::RTCError*)
DECLARE_TO_JS(const webrtc::RTCError*)
DECLARE_CONVERTER(const webrtc::RTCError*, SomeError)
DECLARE_TO_JS(webrtc::RtcpParameters)
DECLARE_TO_JS(webrtc::RtpParameters)
DECLARE_TO_JS(webrtc::RtpExtension)
DECLARE_TO_JS(webrtc::RtpSource)
DECLARE_TO_JS(const webrtc::RTCStats*)
DECLARE_TO_JS(const webrtc::RTCStatsMemberInterface*)
DECLARE_FROM_JS(webrtc::SessionDescriptionInterface*)
DECLARE_CONVERTER(webrtc::SessionDescriptionInterface*, RTCSessionDescriptionInit)
DECLARE_TO_JS(const webrtc::SessionDescriptionInterface*)

typedef std::pair<double, std::vector<std::map<std::string, std::string>>> RTCStatsResponseInit;

DECLARE_TO_AND_FROM_JS(ExtendedRTCConfiguration)
DECLARE_CONVERTER(RTCSessionDescriptionInit, webrtc::SessionDescriptionInterface*)
DECLARE_TO_JS(RTCStatsResponseInit)
DECLARE_TO_JS(webrtc::VideoTrackSourceInterface::Stats)
DECLARE_FROM_JS(rtc::scoped_refptr<webrtc::I420Buffer>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::VideoFrameBuffer>)
DECLARE_TO_JS(rtc::scoped_refptr<webrtc::I420BufferInterface>)
DECLARE_TO_JS(webrtc::VideoFrame)
DECLARE_FROM_JS(node_webrtc::I420ImageData)
DECLARE_FROM_JS(node_webrtc::RgbaImageData)

}  // namespace node_webrtc
