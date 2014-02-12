#ifndef __VOICEMEDIACHANNEL_H__
#define __VOICEMEDIACHANNEL_H__

#include <string>

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "talk/app/webrtc/jsep.h"
#include "talk/media/base/mediachannel.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

class VoiceMediaChannel: public ObjectWrap
{
    
public:
  
  VoiceMediaChannel(cricket::VoiceMediaChannel* VoiceMediaChannel);
  ~VoiceMediaChannel();
  
  //
  // Nodejs wrapping.
  //
  
  static void Init( Handle<Object> exports );
  static Persistent<Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(SetLocalRenderer);
  static NAN_METHOD(addSendStream);

private:
  cricket::VoiceMediaChannel* _internalVoiceMediaChannel;
};

#endif
