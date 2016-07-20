#include "node.h"
#include "v8.h"

#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/thread.h"

#include "peerconnection.h"
#include "datachannel.h"
#include "rtcstatsreport.h"
#include "rtcstatsresponse.h"

using v8::Handle;
using v8::Object;

static rtc::Thread signalingThread;
static rtc::Thread workerThread;

void setup(rtc::Thread* signalingThread, rtc::Thread* workerThread) {
  bool result;

  result = rtc::InitializeSSL();
  assert(result);

  result = signalingThread->Start();
  assert(result);

  result = workerThread->Start();
  assert(result);
}

void dispose(void* args) {
  workerThread.Stop();

  signalingThread.Stop();

  rtc::CleanupSSL();
}

void init(Handle<Object> exports) {
  setup(&signalingThread, &workerThread);
  node_webrtc::PeerConnection::Init(&signalingThread, &workerThread, exports);
  node_webrtc::DataChannel::Init(exports);
  node_webrtc::RTCStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
  node::AtExit(dispose);
}

NODE_MODULE(wrtc, init)
