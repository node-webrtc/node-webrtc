#ifndef __MEDIASTREAM_H__
#define __MEDIASTREAM_H__

#include <queue>
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

class MediaStream
: public Nan::ObjectWrap,
  public webrtc::ObserverInterface
{
    
  // Helper structs/enums
  public:
    enum AsyncEventType {
      ACTIVE = 0x1 << 0, // 1
      ADDTRACK = 0x1 << 1, // 2
      INACTIVE = 0x1 << 2, // 4
      REMOVETRACK = 0x1 << 3, // 8
      CHANGE = 0x1 << 4, // 16
    };
  
  // Class Implementation
  public:
    MediaStream(webrtc::MediaStreamInterface* MediaStream);
    ~MediaStream();

  private:
    static void Run(uv_async_t* handle, int status);
    
    // ObserverInterface implementation.  
  public:
    void OnChanged();
  
    // NodeJS Wrapping
    static void Init(v8::Handle<v8::Object> exports);
    static Nan::Persistent<Function> constructor;
    static NAN_METHOD(New);

    static NAN_METHOD(GetVideoTracks);

    static NAN_GETTER(GetId);

    static NAN_SETTER(ReadOnly);

    static NAN_GETTER(GetState);

  void QueueEvent(MediaStream::AsyncEventType type, void* data);
    webrtc::MediaStreamInterface* GetInterface();
  
  private:
    struct AsyncEvent {
      AsyncEventType type;
      void* data;
    };

    uv_mutex_t lock;
    uv_async_t async;
    uv_loop_t *loop;
    std::queue<AsyncEvent> _events;
 
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _internalMediaStream;
};

}  // namespace node_webrtc

#endif
