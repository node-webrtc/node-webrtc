/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "peerconnectionfactory.h"

#include "webrtc/base/ssladapter.h"
#include "webrtc/modules/audio_device/include/fake_audio_device.h"
#include "webrtc/p2p/base/basicpacketsocketfactory.h"

#include "src/common.h"
#include "src/zerocapturer.h"
#include "src/webrtc/fake_audio_device.h"

using node_webrtc::Maybe;
using node_webrtc::PeerConnectionFactory;
using v8::External;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::Integer;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
using v8::Value;
using v8::Array;
using webrtc::AudioDeviceModule;

Nan::Persistent<Function> PeerConnectionFactory::constructor;
std::shared_ptr<PeerConnectionFactory> PeerConnectionFactory::_default;
uv_mutex_t PeerConnectionFactory::_lock;
int PeerConnectionFactory::_references = 0;

PeerConnectionFactory::PeerConnectionFactory(Maybe<AudioDeviceModule::AudioLayer> audioLayer) {
  TRACE_CALL;

  bool result;
  (void) result;

#if defined(WEBRTC_USE_EPOLL)
  _physicalSocketServer = std::unique_ptr<node_webrtc::PhysicalSocketServer>(new PhysicalSocketServer());
  assert(_physicalSocketServer);

  _workerThread = std::unique_ptr<rtc::Thread>(new rtc::Thread(_physicalSocketServer.get()));
#else
  _workerThread = std::unique_ptr<rtc::Thread>(new rtc::Thread());
#endif
  assert(_workerThread);

  result = _workerThread->Start();
  assert(result);

  _audioDeviceModule = _workerThread->Invoke<rtc::scoped_refptr<AudioDeviceModule>>(RTC_FROM_HERE, [audioLayer]() {
#if defined(WEBRTC_WIN)
    return webrtc::AudioDeviceModule::Create(0,
            audioLayer.FromMaybe(webrtc::AudioDeviceModule::AudioLayer::kDummyAudio));
#else
    return audioLayer.Map([](const webrtc::AudioDeviceModule::AudioLayer audioLayer) {
      return webrtc::AudioDeviceModule::Create(0, audioLayer);
    }).Or([]() {
      return node_webrtc::FakeAudioDevice::Create(
              node_webrtc::ZeroCapturer::Create(48000),
              node_webrtc::FakeAudioDevice::CreateDiscardRenderer(48000));
    });
#endif
  });

  _signalingThread = rtc::Thread::Create();
  assert(_signalingThread);

  result = _signalingThread->Start();
  assert(result);

  _factory = webrtc::CreatePeerConnectionFactory(
          _workerThread.get(),
          _signalingThread.get(),
          _audioDeviceModule.get(),
          nullptr,
          nullptr);
  assert(_factory);

  _networkManager = std::unique_ptr<rtc::NetworkManager>(new rtc::BasicNetworkManager());
  _socketFactory = std::unique_ptr<rtc::PacketSocketFactory>(new rtc::BasicPacketSocketFactory(_workerThread.get()));

  TRACE_END;
}

PeerConnectionFactory::~PeerConnectionFactory() {
  TRACE_CALL;

  _factory = nullptr;

  _workerThread->Stop();
  _signalingThread->Stop();

  _workerThread = nullptr;
  _signalingThread = nullptr;
  _audioDeviceModule = nullptr;

  _networkManager = nullptr;
  _socketFactory = nullptr;

  TRACE_END;
}

NAN_METHOD(PeerConnectionFactory::New) {
  TRACE_CALL;
  if (!info.IsConstructCall()) {
    return Nan::ThrowTypeError("Use the new operator to construct a PeerConnectionFactory.");
  }

  // TODO(mroberts): Read `audioLayer` from some PeerConnectionFactoryOptions?
  auto peerConnectionFactory = new PeerConnectionFactory();
  peerConnectionFactory->Wrap(info.This());

  TRACE_END;
  info.GetReturnValue().Set(info.This());
}

std::shared_ptr<PeerConnectionFactory> PeerConnectionFactory::GetOrCreateDefault() {
  uv_mutex_lock(&_lock);
  _references++;
  if (_references == 1) {
    _default = std::make_shared<PeerConnectionFactory>();
  }
  uv_mutex_unlock(&_lock);
  return _default;
}

void PeerConnectionFactory::Release() {
  uv_mutex_lock(&_lock);
  _references--;
  assert(_references >= 0);
  if (!_references) {
    _default = nullptr;
  }
  uv_mutex_unlock(&_lock);
}

void PeerConnectionFactory::Dispose() {
  uv_mutex_destroy(&_lock);
  rtc::CleanupSSL();
}

void PeerConnectionFactory::Init(Handle<Object> exports) {
  uv_mutex_init(&_lock);

  bool result;
  (void) result;

  result = rtc::InitializeSSL();
  assert(result);

  Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
  tpl->SetClassName(Nan::New("PeerConnectionFactory").ToLocalChecked());
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  constructor.Reset(tpl->GetFunction());
  exports->Set(Nan::New("PeerConnectionFactory").ToLocalChecked(), tpl->GetFunction());
}
