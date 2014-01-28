#ifndef __FILEMEDIAENGINE_H__
#define __FILEMEDIAENGINE_H__

#include <queue>
#include <string>

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "talk/media/base/mediachannel.h"
#include "talk/media/base/filemediaengine.h"
//#include "talk/base/thread.h"
//#include "talk/base/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

class FileMedia: public ObjectWrap
{
    
public:

  enum AsyncEventType {
    PACKET = 0x1 << 0, // 1
    MESSAGE = 0x1 << 1, // 2
  };

  FileMedia(cricket::FileMediaEngine* FileMediaEngine);
  ~FileMedia();
  
  //
  // ObserverInterface implementation.
  //
  
  //
  // Nodejs wrapping.
  //
  
  static void Init( Handle<Object> exports );
  static Persistent<Function> constructor;
  static NAN_METHOD(New);
  virtual void Terminate() {}

  cricket::FileMediaEngine* GetInterface();

  static NAN_METHOD(createChannel);
  static NAN_METHOD(createVideoChannel);
  static NAN_METHOD(createStream);

  static NAN_METHOD(setVoiceInputFilename);
  static NAN_METHOD(setVoiceOutputFilename);
  static NAN_METHOD(setVideoInputFilename);
  static NAN_METHOD(setVideoOutputFilename);

  void QueueEvent(FileMedia::AsyncEventType type, void* data);

private:
  cricket::VoiceMediaChannel* voice_channel_;
  cricket::VideoMediaChannel* video_channel_;

  static void Run(uv_async_t* handle, int status);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  std::queue<AsyncEvent> _events;

  cricket::FileMediaEngine* _internalFileMediaEngine;
};

#endif
/* ex: set tabstop=2 expandtab: */
