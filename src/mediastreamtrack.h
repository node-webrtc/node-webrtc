#ifndef __MEDIASTREAMTRACK_H__
#define __MEDIASTREAMTRACK_H__

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

class MediaStreamTrack
: public Nan::ObjectWrap,
  public webrtc::ObserverInterface
{
    
public:

  enum AsyncEventType {
    MUTE = 0x1 << 0, // 1
    UNMUTE = 0x1 << 1, // 2
    STARTED = 0x1 << 2, // 4
    ENDED = 0x1 << 3, // 8
    CHANGE = 0x1 << 4, // 16
  };
  
  MediaStreamTrack(webrtc::MediaStreamTrackInterface* MediaStreamTrack);
  ~MediaStreamTrack();
  
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

  static NAN_METHOD(stop);
  static NAN_METHOD(clone);
  
  static NAN_GETTER(GetId);
  static NAN_GETTER(GetKind);
  static NAN_GETTER(GetLabel);
  static NAN_GETTER(GetEnabled);
  static NAN_GETTER(GetMuted);
  static NAN_GETTER(GetReadOnly);
  static NAN_GETTER(GetRemote);
  static NAN_GETTER(GetReadyState);
  
  static NAN_SETTER(SetEnabled);
  static NAN_SETTER(ReadOnly);

  void QueueEvent(MediaStreamTrack::AsyncEventType type, void* data);
  webrtc::MediaStreamTrackInterface* GetInterface();
  
private:
  static void Run(uv_async_t* handle, int status);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  std::queue<AsyncEvent> _events;
  
  bool _live;
  bool _muted;
  
  talk_base::scoped_refptr<webrtc::MediaStreamTrackInterface> _internalMediaStreamTrack;
};

#endif