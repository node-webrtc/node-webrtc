/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_PEERCONNECTION_H_
#define SRC_PEERCONNECTION_H_

#include <stdint.h>

#include <string>
#include <queue>

#include "nan.h"
#include "uv.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/datachannelinterface.h"  // IWYU pragma: keep
#include "webrtc/api/jsep.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/statstypes.h"
#include "webrtc/base/scoped_ref_ptr.h"

#include "peerconnectionfactory.h"
#include "audiosink.h"

// Number of samples to deliver to Javascript layer at a time.
#define AUDIO_SAMPLE_DELIVERY_SIZE 2048

namespace node_webrtc {

class CreateOfferObserver;
class CreateAnswerObserver;
class DataChannelObserver;
class SetLocalDescriptionObserver;
class SetRemoteDescriptionObserver;

class PeerConnection
  : public Nan::ObjectWrap
  , public webrtc::PeerConnectionObserver {
 public:

  std::map<std::string, std::shared_ptr<std::vector<int16_t> > > audioSampleBuffers;

  struct ErrorEvent {
    explicit ErrorEvent(const std::string& msg)
      : msg(msg) {}

    std::string msg;
  };

  struct SdpEvent {
    explicit SdpEvent(webrtc::SessionDescriptionInterface* sdp) {
      if (!sdp->ToString(&desc)) {
        desc = "";
      }
      type = sdp->type();
    }

    std::string type;
    std::string desc;
  };

  struct IceEvent {
    explicit IceEvent(const webrtc::IceCandidateInterface* ice_candidate)
      : sdpMLineIndex(ice_candidate->sdp_mline_index())
      , sdpMid(ice_candidate->sdp_mid()) {
      ice_candidate->ToString(&candidate);
    }

    uint32_t sdpMLineIndex;
    std::string sdpMid;
    std::string candidate;
  };

  struct VideoFrameEvent {
    explicit VideoFrameEvent(std::string label)
        : label(label) {}
    rtc::scoped_refptr<webrtc::VideoFrameBuffer> buffer;
    int width;
    int height;

    std::string label;
  };

  struct AudioFrameEvent {
    explicit AudioFrameEvent(std::string label, int bits_per_sample, int sample_rate,
                             size_t number_of_channels, size_t number_of_frames)
        : label(label), bits_per_sample(bits_per_sample),
          sample_rate(sample_rate), number_of_channels(number_of_channels),
          number_of_frames(number_of_frames) {}
    std::vector<float> buffer;
    int bits_per_sample;
    int sample_rate;
    size_t number_of_channels;
    size_t number_of_frames;

    std::string label;
  };

  struct EncodedVideoFrameEvent {
    explicit EncodedVideoFrameEvent(std::string label)
        : label(label) {}
    std::vector<uint8_t> buffer;
    int width;
    int height;
    int frameType;
    uint64_t timestamp;

    std::string label;
  };

  struct StateEvent {
    explicit StateEvent(uint32_t state)
      : state(state) {}

    uint32_t state;
  };

  struct DataChannelEvent {
    explicit DataChannelEvent(DataChannelObserver* observer)
      : observer(observer) {}

    DataChannelObserver* observer;
  };

  struct GetStatsEvent {
    GetStatsEvent(Nan::Callback* callback, webrtc::StatsReports reports)
      : callback(callback), reports(reports) {}

    Nan::Callback* callback;
    webrtc::StatsReports reports;
  };

  enum AsyncEventType {
    CREATE_OFFER_SUCCESS = 0x1 << 0,  // 1
    CREATE_OFFER_ERROR = 0x1 << 1,  // 2
    CREATE_ANSWER_SUCCESS = 0x1 << 2,  // 4
    CREATE_ANSWER_ERROR = 0x1 << 3,  // 8
    SET_LOCAL_DESCRIPTION_SUCCESS = 0x1 << 4,  // 16
    SET_LOCAL_DESCRIPTION_ERROR = 0x1 << 5,  // 32
    SET_REMOTE_DESCRIPTION_SUCCESS = 0x1 << 6,  // 64
    SET_REMOTE_DESCRIPTION_ERROR = 0x1 << 7,  // 128
    ADD_ICE_CANDIDATE_SUCCESS = 0x1 << 8,  // 256
    ADD_ICE_CANDIDATE_ERROR = 0x1 << 9,  // 512
    NOTIFY_DATA_CHANNEL = 0x1 << 10,  // 1024
    NOTIFY_CONNECTION = 0x1 << 11,  // 2048
    NOTIFY_CLOSED_CONNECTION = 0x1 << 12,  // 4096
    ICE_CANDIDATE = 0x1 << 13,  // 8192
    SIGNALING_STATE_CHANGE = 0x1 << 14,  // 16384
    ICE_CONNECTION_STATE_CHANGE = 0x1 << 15,  // 32768
    ICE_GATHERING_STATE_CHANGE = 0x1 << 16,  // 65536
    NOTIFY_ADD_STREAM = 0x1 << 17,  // 131072
    NOTIFY_REMOVE_STREAM = 0x1 << 18,  // 262144
    GET_STATS_SUCCESS = 0x1 << 19,  // 524288
    NOTIFY_ON_FRAME = 0x1 << 20,
    NOTIFY_ON_ENCODED_FRAME = 0x1 << 21,
    NEGOTIATION_NEEDED = 0x1 << 22,
    ADD_STREAM_SUCCESS = 0x1 << 23,
    NOTIFY_ON_AUDIO_FRAME = 0x1 << 24,
    REGISTER_SINK_AUDIO_FRAME = 0x1 << 25,

    ERROR_EVENT = CREATE_OFFER_ERROR | CREATE_ANSWER_ERROR |
        SET_LOCAL_DESCRIPTION_ERROR | SET_REMOTE_DESCRIPTION_ERROR |
        ADD_ICE_CANDIDATE_ERROR,
    SDP_EVENT = CREATE_OFFER_SUCCESS | CREATE_ANSWER_SUCCESS,
    VOID_EVENT = SET_LOCAL_DESCRIPTION_SUCCESS | SET_REMOTE_DESCRIPTION_SUCCESS |
        ADD_ICE_CANDIDATE_SUCCESS | GET_STATS_SUCCESS | ADD_STREAM_SUCCESS,
    STATE_EVENT = SIGNALING_STATE_CHANGE | ICE_CONNECTION_STATE_CHANGE |
        ICE_GATHERING_STATE_CHANGE
  };

  explicit PeerConnection(webrtc::PeerConnectionInterface::IceServers iceServerList, rtc::Optional<bool> enableDtlsSrtp);
  ~PeerConnection();

  //
  // PeerConnectionObserver implementation.
  //

  virtual void OnError();

  virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state);
  virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state);
  virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state);
  virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);
  virtual void OnRenegotiationNeeded();

  virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel);

  virtual void OnAddStream(webrtc::MediaStreamInterface* stream);
  virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(CreateOffer);
  static NAN_METHOD(CreateAnswer);
  static NAN_METHOD(SetLocalDescription);
  static NAN_METHOD(SetRemoteDescription);
  static NAN_METHOD(UpdateIce);
  static NAN_METHOD(AddIceCandidate);
  static NAN_METHOD(CreateDataChannel);
  /*
  static NAN_METHOD(GetLocalStreams);
  static NAN_METHOD(GetRemoteStreams);
  static NAN_METHOD(GetStreamById);
  static NAN_METHOD(AddStream);
  static NAN_METHOD(RemoveStream);
  */
  static NAN_METHOD(OnStreamAudioFrame);
  static NAN_METHOD(OnStreamVideoFrame);
  static NAN_METHOD(OnStreamEncodedVideoFrame);
  static NAN_METHOD(AddStream);
  static NAN_METHOD(GetStats);
  static NAN_METHOD(Close);

  static NAN_GETTER(GetLocalDescription);
  static NAN_GETTER(GetRemoteDescription);
  static NAN_GETTER(GetIceConnectionState);
  static NAN_GETTER(GetSignalingState);
  static NAN_GETTER(GetIceGatheringState);
  static NAN_SETTER(ReadOnly);

  void QueueEvent(AsyncEventType type, void* data);

 private:
  static void Run(uv_async_t* handle, int status);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  uv_loop_t* loop;
  std::queue<AsyncEvent> _events;
  webrtc::PeerConnectionInterface::IceServers _iceServers;

  rtc::scoped_refptr<CreateOfferObserver> _createOfferObserver;
  rtc::scoped_refptr<CreateAnswerObserver> _createAnswerObserver;
  rtc::scoped_refptr<SetLocalDescriptionObserver> _setLocalDescriptionObserver;
  rtc::scoped_refptr<SetRemoteDescriptionObserver> _setRemoteDescriptionObserver;

  rtc::scoped_refptr<webrtc::PeerConnectionInterface> _jinglePeerConnection;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _jinglePeerConnectionFactory;

  std::shared_ptr<node_webrtc::PeerConnectionFactory> _factory;
  bool _shouldReleaseFactory;
};

}  // namespace node_webrtc

#endif  // SRC_PEERCONNECTION_H_
