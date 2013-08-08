#include <node.h>
#include <v8.h>

#include "xpcom-config.h"
#include "peerconnection.h"

using namespace v8;

void init(Handle<Object> exports) {
  PeerConnection::Init( exports );
}

NODE_MODULE(webrtc, init)