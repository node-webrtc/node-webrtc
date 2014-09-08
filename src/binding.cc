#include <node.h>
#include <v8.h>

#include "talk/base/ssladapter.h"

#include "peerconnection.h"
#include "datachannel.h"
#include "mediastream.h"
#include "mediastreamtrack.h"
#include "getusermedia.h"

void init(v8::Handle<v8::Object> exports) {
  talk_base::InitializeSSL();
  node_webrtc::PeerConnection::Init(exports);
  node_webrtc::DataChannel::Init(exports);
  MediaStream::Init(exports);
  MediaStreamTrack::Init(exports);
  exports->Set( String::NewSymbol("getUserMedia"),
    FunctionTemplate::New( GetUserMedia )->GetFunction() );
}

NODE_MODULE(wrtc, init)
