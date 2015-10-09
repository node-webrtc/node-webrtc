#ifndef __MEDIASTREAM_H__
#define __MEDIASTREAM_H__

#include <queue>
#include <string>

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

class MediaStream
: public Nan::ObjectWrap,
  public webrtc::ObserverInterface
{
    
public:

  enum AsyncEventType {
    ACTIVE = 0x1 << 0, // 1
    ADDTRACK = 0x1 << 1, // 2
    INACTIVE = 0x1 << 2, // 4
    REMOVETRACK = 0x1 << 3, // 8
    CHANGE = 0x1 << 4, // 16
  };
  
  MediaStream(webrtc::MediaStreamInterface* MediaStream);
  ~MediaStream();
  
  //
  // ObserverInterface implementation.
  //
  
  void OnChanged();
  
  //
  // Nodejs wrapping.
  //
  
  static void Init( Handle<Object> exports );
  static Nan::Persistent<Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(getAudioTracks);
  static NAN_METHOD(getVideoTracks);
  static NAN_METHOD(getTrackById);
  static NAN_METHOD(addTrack);
  static NAN_METHOD(removeTrack);
  static NAN_METHOD(clone);
  
  static NAN_GETTER(GetId);
  static NAN_GETTER(IsInactive);
  
  static NAN_SETTER(ReadOnly);

  void QueueEvent(MediaStream::AsyncEventType type, void* data);
  webrtc::MediaStreamInterface* GetInterface();
  bool IsMediaStreamActive();
  
private:
  static void Run(uv_async_t* handle, int status);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  std::queue<AsyncEvent> _events;

  bool _inactive;
  
  talk_base::scoped_refptr<webrtc::MediaStreamInterface> _internalMediaStream;
};

#endif