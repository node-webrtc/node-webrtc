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
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

class DataChannel;

class _DataChannelObserver
  : public webrtc::DataChannelObserver
{
  public:
    _DataChannelObserver(talk_base::scoped_refptr<webrtc::DataChannelInterface> jingleDataChannel);
    virtual ~_DataChannelObserver();

    virtual void OnStateChange();
    virtual void OnMessage(const webrtc::DataBuffer& buffer);

    talk_base::scoped_refptr<webrtc::DataChannelInterface> GetJingleDataChannel();
    void SetJsDataChannel(DataChannel* js_data_channel);

  private:
    talk_base::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
    DataChannel* _jsDataChannel;
};

class DataChannel
: public ObjectWrap
, public webrtc::DataChannelObserver
{

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

  DataChannel(_DataChannelObserver* observer);
  ~DataChannel();

  //
  // DataChannelObserver implementation.
  //

  virtual void OnStateChange();
  virtual void OnMessage(const webrtc::DataBuffer& buffer);

  //
  // Nodejs wrapping.
  //
  static void Init( Handle<Object> exports );
  static Persistent<Function> constructor;
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

  talk_base::scoped_refptr<webrtc::DataChannelInterface> _jingleDataChannel;
  DataChannelObserver* _observer;
  BinaryType _binaryType;

  static Persistent<Function> ArrayBufferConstructor;
};

#endif
