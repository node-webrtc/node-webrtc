/* Copyright (c) 2017 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "node.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/ssladapter.h"

#include "src/peerconnection.h"
#include "src/datachannel.h"
#include "src/rtcstatsreport.h"
#include "src/rtcstatsresponse.h"
#ifdef DEBUG
#include "test.h"
#endif

using v8::Handle;
using v8::Object;

static rtc::Thread* signalingThread = nullptr;
static rtc::Thread* workerThread = nullptr;

void dispose(void*) {
  node_webrtc::PeerConnection::Dispose();
  node_webrtc::DataChannel::Dispose();
  node_webrtc::RTCStatsReport::Dispose();
  node_webrtc::RTCStatsResponse::Dispose();

  workerThread->Stop();
  delete workerThread;

  signalingThread->Stop();
  delete signalingThread;

  rtc::CleanupSSL();
}

void init(Handle<Object> exports) {
  bool result;
  (void) result;

  result = rtc::InitializeSSL();
  assert(result);

  signalingThread = new rtc::Thread();
  result = signalingThread->Start();
  assert(result);

  workerThread = new rtc::Thread();
  result = workerThread->Start();
  assert(result);

  node_webrtc::PeerConnection::Init(signalingThread, workerThread, exports);
  node_webrtc::DataChannel::Init(exports);
  node_webrtc::RTCStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
#ifdef DEBUG
  node_webrtc::Test::Init(exports);
#endif

  node::AtExit(dispose);
}

NODE_MODULE(wrtc, init)
