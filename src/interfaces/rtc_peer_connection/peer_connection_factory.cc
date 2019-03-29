/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peer_connection_factory.h"

#include <memory>

#include <uv.h>
#include <webrtc/api/create_peerconnection_factory.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/audio_codecs/builtin_audio_decoder_factory.h>
#include <webrtc/api/audio_codecs/builtin_audio_encoder_factory.h>
#include <webrtc/api/video_codecs/builtin_video_decoder_factory.h>
#include <webrtc/api/video_codecs/builtin_video_encoder_factory.h>
#include <webrtc/api/video_codecs/video_decoder_factory.h>
#include <webrtc/api/video_codecs/video_encoder_factory.h>
#include <webrtc/modules/audio_device/include/audio_device.h>
#include <webrtc/modules/audio_device/include/fake_audio_device.h>
#include <webrtc/p2p/base/basic_packet_socket_factory.h>
#include <webrtc/rtc_base/location.h>
#include <webrtc/rtc_base/ssl_adapter.h>
#include <webrtc/rtc_base/thread.h>

#include "src/webrtc/test_audio_device_module.h"
#include "src/webrtc/zero_capturer.h"

Nan::Persistent<v8::Function>& node_webrtc::PeerConnectionFactory::constructor() {
  static Nan::Persistent<v8::Function> constructor;
  return constructor;
}

std::shared_ptr<node_webrtc::PeerConnectionFactory> node_webrtc::PeerConnectionFactory::_default;
uv_mutex_t node_webrtc::PeerConnectionFactory::_lock;
int node_webrtc::PeerConnectionFactory::_references = 0;

node_webrtc::PeerConnectionFactory::PeerConnectionFactory(node_webrtc::Maybe<webrtc::AudioDeviceModule::AudioLayer> audioLayer) {
  _workerThread = std::make_unique<rtc::Thread>();
  assert(_workerThread);

  bool result = _workerThread->Start();
  assert(result);

  _audioDeviceModule = _workerThread->Invoke<rtc::scoped_refptr<webrtc::AudioDeviceModule>>(RTC_FROM_HERE, [audioLayer]() {
    return audioLayer.Map([](const webrtc::AudioDeviceModule::AudioLayer audioLayer) {
      return webrtc::AudioDeviceModule::Create(0, audioLayer);
    }).Or([]() {
      return node_webrtc::TestAudioDeviceModule::CreateTestAudioDeviceModule(
              node_webrtc::ZeroCapturer::Create(48000),
              node_webrtc::TestAudioDeviceModule::CreateDiscardRenderer(48000));
    });
  });

  _signalingThread = rtc::Thread::Create();
  assert(_signalingThread);

  result = _signalingThread->Start();
  assert(result);

  _factory = webrtc::CreatePeerConnectionFactory(
          _workerThread.get(),
          _workerThread.get(),
          _signalingThread.get(),
          _audioDeviceModule.get(),
          webrtc::CreateBuiltinAudioEncoderFactory(),
          webrtc::CreateBuiltinAudioDecoderFactory(),
          webrtc::CreateBuiltinVideoEncoderFactory(),
          webrtc::CreateBuiltinVideoDecoderFactory(),
          nullptr,
          nullptr);
  assert(_factory);

  _networkManager = std::unique_ptr<rtc::NetworkManager>(new rtc::BasicNetworkManager());
  _socketFactory = std::unique_ptr<rtc::PacketSocketFactory>(new rtc::BasicPacketSocketFactory(_workerThread.get()));
}

node_webrtc::PeerConnectionFactory::~PeerConnectionFactory() {
  _factory = nullptr;

  _workerThread->Invoke<void>(RTC_FROM_HERE, [this]() {
    this->_audioDeviceModule = nullptr;
  });

  _workerThread->Stop();
  _signalingThread->Stop();

  _workerThread = nullptr;
  _signalingThread = nullptr;

  _networkManager = nullptr;
  _socketFactory = nullptr;
}

NAN_METHOD(node_webrtc::PeerConnectionFactory::New) {
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct a PeerConnectionFactory.");
  }

  // TODO(mroberts): Read `audioLayer` from some PeerConnectionFactoryOptions?
  auto peerConnectionFactory = new node_webrtc::PeerConnectionFactory();
  peerConnectionFactory->Wrap(info.This());

  info.GetReturnValue().Set(info.This());
}

std::shared_ptr<node_webrtc::PeerConnectionFactory> node_webrtc::PeerConnectionFactory::GetOrCreateDefault() {
  uv_mutex_lock(&_lock);
  _references++;
  if (_references == 1) {
    _default = std::make_shared<node_webrtc::PeerConnectionFactory>();
  }
  uv_mutex_unlock(&_lock);
  return _default;
}

void node_webrtc::PeerConnectionFactory::Release() {
  uv_mutex_lock(&_lock);
  _references--;
  assert(_references >= 0);
  if (!_references) {
    _default = nullptr;
  }
  uv_mutex_unlock(&_lock);
}

void node_webrtc::PeerConnectionFactory::Dispose() {
  uv_mutex_destroy(&_lock);
  rtc::CleanupSSL();
}

void node_webrtc::PeerConnectionFactory::Init(v8::Handle<v8::Object> exports) {
  uv_mutex_init(&_lock);

  bool result;
  (void) result;

  result = rtc::InitializeSSL();
  assert(result);

  v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnectionFactory").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor().Reset(tpl->GetFunction());
  exports->Set(Nan::New("PeerConnectionFactory").ToLocalChecked(), tpl->GetFunction());
}
