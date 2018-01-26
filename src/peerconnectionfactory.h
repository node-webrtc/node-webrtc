/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_PEERCONNECTIONFACTORY_H_
#define SRC_PEERCONNECTIONFACTORY_H_

#include "nan.h"
#include "uv.h"
#include "v8.h"  // IWYU pragma: keep

#include "webrtc/api/peerconnectionfactory.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/modules/audio_device/include/audio_device.h"
#include "decoderfactory.h"

namespace node_webrtc {

class PeerConnectionFactory
  : public Nan::ObjectWrap {
 public:
  /**
   * Create a PeerConnectionFactory using a particular webrtc::AudioDeviceModule.
   */
  explicit PeerConnectionFactory(rtc::scoped_refptr<webrtc::AudioDeviceModule> audioDeviceModule);

  /**
   * Create a PeerConnectionFactory using a particular webrtc::AudioDeviceModule::AudioLayer.
   */
  explicit PeerConnectionFactory(webrtc::AudioDeviceModule::AudioLayer audioLayer)
    : PeerConnectionFactory(webrtc::AudioDeviceModule::Create(0, audioLayer)) {}

  /**
   * Create a PeerConnectionFactory using webrtc::AudioDeviceModule::AudioLayer::kDummyAudio.
   */
  PeerConnectionFactory(): PeerConnectionFactory(webrtc::AudioDeviceModule::kDummyAudio) {}

  ~PeerConnectionFactory() override;

  /**
   * Get or create the default PeerConnectionFactory. The default uses
   * webrtc::AudioDeviceModule::AudioLayer::kDummyAudio. Call {@link Release} when done.
   */
  static std::shared_ptr<PeerConnectionFactory> GetOrCreateDefault();

  /**
   * Release a reference to the default PeerConnectionFactory.
   */
  static void Release();

  /**
   * Get the underlying webrtc::PeerConnectionFactoryInterface.
   */
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory() { return _factory; }

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static void Dispose();
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

 private:
  NodeDecoderFactory* _decoderFactory;

  static std::shared_ptr<PeerConnectionFactory> _default;
  static uv_mutex_t _lock;
  static int _references;

  std::unique_ptr<rtc::Thread> _signalingThread;
  std::unique_ptr<rtc::Thread> _workerThread;
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _factory;
};

}  // namespace node_webrtc

#endif  // SRC_PEERCONNECTIONFACTORY_H_
