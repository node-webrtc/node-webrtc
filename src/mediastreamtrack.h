#ifndef __MEDIASTREAMTRACK_H__
#define __MEDIASTREAMTRACK_H__

#include <string>

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/api/jsep.h"
#include "webrtc/api/mediastreaminterface.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

namespace node_webrtc {

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
  
  // Class implementation
  MediaStreamTrack(webrtc::MediaStreamTrackInterface* MediaStreamTrack);
  ~MediaStreamTrack();

  void QueueEvent(MediaStreamTrack::AsyncEventType type, void* data);
  webrtc::MediaStreamTrackInterface* GetInterface();

  // ObserverInterface implementation
  void OnChanged();

  // NodeJS Wrapping
  static void Init(Handle<Object> exports);
  static Nan::Persistent<Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(stop);
  static NAN_METHOD(clone);

  static NAN_GETTER(GetId);
  static NAN_GETTER(GetLabel);
  static NAN_GETTER(GetKind);
  static NAN_GETTER(GetEnabled);
  static NAN_GETTER(GetMuted);
  static NAN_GETTER(GetReadOnly);
  static NAN_GETTER(GetRemote);
  static NAN_GETTER(GetReadyState);

  static NAN_SETTER(SetEnabled);
  static NAN_SETTER(ReadOnly);

private:
  static void Run(uv_async_t* handle, int status);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  uv_loop_t *loop;
  std::queue<AsyncEvent> _events;

  bool _live;
  bool _muted;

  rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _internalMediaStreamTrack;
};

}  // namespace node_webrtc

#endif
