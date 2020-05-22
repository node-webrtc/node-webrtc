/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peer_connection_factory.h"

#include <memory>

#include <webrtc/api/audio_codecs/builtin_audio_decoder_factory.h>
#include <webrtc/api/audio_codecs/builtin_audio_encoder_factory.h>
#include <webrtc/api/create_peerconnection_factory.h>
#include <webrtc/api/peer_connection_interface.h>
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

namespace node_webrtc {

Napi::FunctionReference& PeerConnectionFactory::constructor() {
  static Napi::FunctionReference constructor;
  return constructor;
}

PeerConnectionFactory* PeerConnectionFactory::_default = nullptr;
std::mutex PeerConnectionFactory::_mutex{};  // NOLINT
int PeerConnectionFactory::_references = 0;

PeerConnectionFactory::PeerConnectionFactory(const Napi::CallbackInfo& info)
  : Napi::ObjectWrap<PeerConnectionFactory>(info) {
  auto env = info.Env();

  if (!info.IsConstructCall()) {
    Napi::TypeError::New(env, "Use the new operator to construct a PeerConnectionFactory.").ThrowAsJavaScriptException();
    return;
  }

  // TODO(mroberts): Read `audioLayer` from some PeerConnectionFactoryOptions?
  auto audioLayer = MakeNothing<webrtc::AudioDeviceModule::AudioLayer>();

  _workerThread = rtc::Thread::CreateWithSocketServer();
  assert(_workerThread);

  bool result = _workerThread->SetName("PeerConnectionFactory:workerThread", nullptr);
  assert(result);

  result = _workerThread->Start();
  assert(result);

  _audioDeviceModule = _workerThread->Invoke<rtc::scoped_refptr<webrtc::AudioDeviceModule>>(RTC_FROM_HERE, [audioLayer]() {
    return audioLayer.Map([](auto audioLayer) {
      // TODO(mroberts): I'm just trying to get this to compile right now.
      // We need to call something like CreateDefaultTaskQueueFactory().
      // This code is currently unused, though.
      return webrtc::AudioDeviceModule::Create(audioLayer, nullptr);
    }).Or([]() {
      return TestAudioDeviceModule::CreateTestAudioDeviceModule(
              ZeroCapturer::Create(48000),
              TestAudioDeviceModule::CreateDiscardRenderer(48000));
    });
  });

  _signalingThread = rtc::Thread::Create();
  assert(_signalingThread);

  result = _signalingThread->SetName("PeerConnectionFactory:signalingThread", nullptr);
  assert(result);

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

  webrtc::PeerConnectionFactoryInterface::Options options;
  options.network_ignore_mask = 0;
  _factory->SetOptions(options);

  _networkManager = std::unique_ptr<rtc::NetworkManager>(new rtc::BasicNetworkManager());
  assert(_networkManager != nullptr);

  _socketFactory = std::unique_ptr<rtc::PacketSocketFactory>(new rtc::BasicPacketSocketFactory(_workerThread.get()));
  assert(_socketFactory != nullptr);
}

PeerConnectionFactory::~PeerConnectionFactory() {
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

PeerConnectionFactory* PeerConnectionFactory::GetOrCreateDefault() {
  _mutex.lock();
  _references++;
  if (_references == 1) {
    assert(_default == nullptr);
    auto env = constructor().Env();
    Napi::HandleScope scope(env);
    auto object = constructor().New({});
    auto factory = Unwrap(object);
    _default = factory;
    _default->Ref();
  }
  _mutex.unlock();
  return _default;
}

void PeerConnectionFactory::Release() {
  _mutex.lock();
  _references--;
  assert(_references >= 0);
  if (!_references) {
    assert(_default != nullptr);
    _default->Unref();
    _default = nullptr;
  }
  _mutex.unlock();
}

void PeerConnectionFactory::Dispose() {
  rtc::CleanupSSL();
}

void PeerConnectionFactory::Init(Napi::Env env, Napi::Object exports) {
  bool result;
  (void) result;

  result = rtc::InitializeSSL();
  assert(result);

  auto func = DefineClass(env, "RTCPeerConnectionFactory", {});

  constructor() = Napi::Persistent(func);
  constructor().SuppressDestruct();

  exports.Set("RTCPeerConnectionFactory", func);
}

}  // namespace node_webrtc
