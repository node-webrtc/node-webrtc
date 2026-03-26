/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#pragma once

#include <memory>
#include <mutex>

#include <node-addon-api/napi.h>
#include <webrtc/api/peer_connection_interface.h>
#include <webrtc/api/scoped_refptr.h>
#include <webrtc/modules/audio_device/include/audio_device.h>
#include <webrtc/rtc_base/thread.h>

namespace node_webrtc {

class PeerConnectionFactory : public Napi::ObjectWrap<PeerConnectionFactory> {
public:
  PeerConnectionFactory(const PeerConnectionFactory &) = delete;
  PeerConnectionFactory(PeerConnectionFactory &&) = delete;
  PeerConnectionFactory &operator=(const PeerConnectionFactory &) = delete;
  PeerConnectionFactory &operator=(PeerConnectionFactory &&) = delete;
  explicit PeerConnectionFactory(const Napi::CallbackInfo &);

  ~PeerConnectionFactory() override;

  /**
   * Get or create the default PeerConnectionFactory. The default uses
   * webrtc::AudioDeviceModule::AudioLayer::kDummyAudio. Call {@link Release}
   * when done.
   */
  static PeerConnectionFactory *GetOrCreateDefault();

  /**
   * Release a reference to the default PeerConnectionFactory.
   */
  static void Release();

  /**
   * Get the underlying webrtc::PeerConnectionFactoryInterface.
   */
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory() {
    return _factory;
  }

  std::unique_ptr<rtc::Thread> &SignalingThread() { return _signalingThread; }

  std::unique_ptr<rtc::Thread> &WorkerThread() { return _workerThread; }

  rtc::NetworkManager *getNetworkManager() { return _networkManager.get(); }

  rtc::PacketSocketFactory *getSocketFactory() { return _socketFactory.get(); }

  static void Init(Napi::Env, Napi::Object);

  static Napi::FunctionReference &constructor();

  static void Dispose();

private:
  std::unique_ptr<rtc::Thread> _signalingThread;
  std::unique_ptr<rtc::Thread> _workerThread;

  static PeerConnectionFactory *_default; // NOLINT
  static std::mutex _mutex;               // NOLINT
  static int _references;                 // NOLINT

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> _factory;
  rtc::scoped_refptr<webrtc::AudioDeviceModule> _audioDeviceModule;

  std::unique_ptr<rtc::NetworkManager> _networkManager;
  std::unique_ptr<rtc::PacketSocketFactory> _socketFactory;
};

} // namespace node_webrtc
