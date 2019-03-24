/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_PEERCONNECTIONFACTORY_H_
#define SRC_PEERCONNECTIONFACTORY_H_

#include <memory>

#include <nan.h>
#include <uv.h>  // IWYU pragma: keep
#include <webrtc/modules/audio_device/include/audio_device.h>
#include <webrtc/rtc_base/scoped_ref_ptr.h>
#include <v8.h>  // IWYU pragma: keep

#include "src/functional/maybe.h"  // IWYU pragma: keep

// IWYU pragma: no_include <uv/unix.h>

namespace rtc {

class NetworkManager;
class PacketSocketFactory;
class Thread;  // IWYU pragma: keep

}  // namespace rtc

namespace webrtc {

class PeerConnectionFactoryInterface;

}  // namespace webrtc

namespace node_webrtc {

class PeerConnectionFactory
  : public Nan::ObjectWrap {
 public:
  /**
   * Create a PeerConnectionFactory using a particular webrtc::AudioDeviceModule::AudioLayer.
   */
  explicit PeerConnectionFactory(
      Maybe<webrtc::AudioDeviceModule::AudioLayer> audioLayer = Maybe<webrtc::AudioDeviceModule::AudioLayer>::Nothing());

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

  rtc::NetworkManager* getNetworkManager() { return _networkManager.get(); }

  rtc::PacketSocketFactory* getSocketFactory() { return _socketFactory.get(); }

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);

  static void Dispose();

  std::unique_ptr<rtc::Thread> _signalingThread;
  std::unique_ptr<rtc::Thread> _workerThread;

 private:
  static Nan::Persistent<v8::Function>& constructor();

  static NAN_METHOD(New);

  static std::shared_ptr<PeerConnectionFactory> _default;
  static uv_mutex_t _lock;
  static int _references;

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _factory;
  rtc::scoped_refptr<webrtc::AudioDeviceModule> _audioDeviceModule;

  std::unique_ptr<rtc::NetworkManager> _networkManager;
  std::unique_ptr<rtc::PacketSocketFactory> _socketFactory;
};

}  // namespace node_webrtc

#endif  // SRC_PEERCONNECTIONFACTORY_H_
