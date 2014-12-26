#ifndef __DATACHANNEL_H__
#define __DATACHANNEL_H__

#include <queue>
#include <string>

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "nan.h"

namespace node_webrtc {

class DataChannelObserver;

class DataChannel
: public node::ObjectWrap
, public webrtc::DataChannelObserver
{

  friend class node_webrtc::DataChannelObserver;

public:

  struct ErrorEvent {
    ErrorEvent(const std::string& msg)
    : msg(msg) {}

    std::string msg;
  };

  struct MessageEvent {
    MessageEvent(const webrtc::DataBuffer* buffer)
    {
      binary = buffer->binary;
      size = buffer->size();
      message = new char[size];
      memcpy(static_cast<void*>(message), static_cast<const void*>(buffer->data.data()), size);
    }

    bool binary;
    char* message;
    size_t size;
  };

  struct StateEvent {
    StateEvent(const webrtc::DataChannelInterface::DataState state)
    : state(state) {}

    webrtc::DataChannelInterface::DataState state;
  };

  enum AsyncEventType {
    MESSAGE = 0x1 << 0, // 1
    ERROR = 0x1 << 1, // 2
    STATE = 0x1 << 2, // 4
  };

  enum BinaryType {
    BLOB = 0x0,
    ARRAY_BUFFER = 0x1
  };

  DataChannel(node_webrtc::DataChannelObserver* observer);
  ~DataChannel();

  //
  // DataChannelObserver implementation.
  //

  virtual void OnStateChange();
  virtual void OnMessage(const webrtc::DataBuffer& buffer);

  //
  // Nodejs wrapping.
  //
  static void Init(v8::Handle<v8::Object> exports);
  static v8::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(Send);
  static NAN_METHOD(Close);
  static NAN_METHOD(Shutdown);

  static NAN_GETTER(GetLabel);
  static NAN_GETTER(GetBinaryType);
  static NAN_GETTER(GetReadyState);
  static NAN_SETTER(SetBinaryType);
  static NAN_SETTER(ReadOnly);

  void QueueEvent(DataChannel::AsyncEventType type, void* data);

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

  rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
  DataChannelObserver* _observer;
  BinaryType _binaryType;

#if NODE_MODULE_VERSION < 0x000C
  static v8::Persistent<v8::Function> ArrayBufferConstructor;
#endif

};

class DataChannelObserver
  : public webrtc::DataChannelObserver
{
  public:
    DataChannelObserver(rtc::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel);
    virtual ~DataChannelObserver();

    virtual void OnStateChange();
    virtual void OnMessage(const webrtc::DataBuffer& buffer);
    void QueueEvent(DataChannel::AsyncEventType type, void* data);

    uv_mutex_t lock;
    std::queue<DataChannel::AsyncEvent> _events;
    rtc::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
};

}

#endif
