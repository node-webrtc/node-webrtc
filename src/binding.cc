#include <node.h>
#include <v8.h>

#include "talk/base/ssladapter.h"

#include "peerconnection.h"
#include "datachannel.h"
//#include "mediastream.h"
//#include "mediastreamtrack.h"

using namespace v8;

void init(Handle<Object> exports) {
  talk_base::InitializeSSL();
  PeerConnection::Init(exports);
  DataChannel::Init(exports);
  //MediaStream::Init(exports);
  //MediaStreamTrack::Init(exports);
}

NODE_MODULE(wrtc, init)