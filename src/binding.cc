#include "node.h"
#include "v8.h"

#include "webrtc/base/ssladapter.h"

#include "peerconnection.h"
#include "datachannel.h"
#include "rtcstatsreport.h"
#include "rtcstatsresponse.h"

using v8::Handle;
using v8::Object;

void init(Handle<Object> exports) {
  rtc::InitializeSSL();
  node_webrtc::PeerConnection::Init(exports);
  node_webrtc::DataChannel::Init(exports);
  node_webrtc::RTCStatsReport::Init(exports);
  node_webrtc::RTCStatsResponse::Init(exports);
}

NODE_MODULE(wrtc, init)
