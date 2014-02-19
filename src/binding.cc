#include <stdarg.h>

#include <node.h>
#include <v8.h>

#include "talk/base/ssladapter.h"

#include "peerconnection.h"
#include "datachannel.h"
#include "mediastream.h"
#include "mediastreamtrack.h"

using namespace v8;

bool doTrace = false;

void setTracing (bool trace)
{
  doTrace = trace;
}

void tracePrint (std::string format, ...)
{
  if (!doTrace)
    return;

  va_list args;
  va_start(args, format);
  vfprintf(stdout, format.c_str(), args);
  va_end(args);
}

Handle<Value> SetTracing(const Arguments &args)
{
  HandleScope scope;
  doTrace = args[0]->BooleanValue();
  return scope.Close(Integer::New(0));
}

void init(Handle<Object> exports) {
  talk_base::InitializeSSL();
  PeerConnection::Init(exports);
  DataChannel::Init(exports);
  MediaStream::Init(exports);
  MediaStreamTrack::Init(exports);

  Handle<FunctionTemplate> setTraceTpl =
    FunctionTemplate::New(SetTracing);
  exports->Set(String::New("setTracing"), 
               setTraceTpl->GetFunction());
}

NODE_MODULE(webrtc, init)
