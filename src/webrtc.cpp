#include <node.h>
#include <v8.h>

using namespace v8;

Handle<Value> PeerConnection(const Arguments& args) {
  HandleScope scope;
  return scope.Close(String::New("this is a PeerConnection"));
}

void init(Handle<Object> exports) {
  exports->Set(String::NewSymbol("PeerConnection"),
      FunctionTemplate::New(PeerConnection)->GetFunction());
}

NODE_MODULE(webrtc, init)