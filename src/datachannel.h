#ifndef __DATACHANNEL_H__
#define __DATACHANNEL_H__

#include <queue>
#include <string>

#include <v8.h>
#include <node.h>
#include <uv.h>

#include "talk/app/webrtc/jsep.h"
#include "talk/app/webrtc/datachannelinterface.h"
#include "talk/base/thread.h"
#include "talk/base/scoped_ptr.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"

using namespace node;
using namespace v8;

class DataChannel
: public ObjectWrap,
  public webrtc::DataChannelObserver
{

public:

  struct ErrorEvent {
    ErrorEvent(const std::string& msg)
    : msg(msg) {}

    std::string msg;
  };

  enum AsyncEventType {
    MESSAGE = 0x1 << 0, // 1
    ERROR = 0x1 << 1, // 2
    STATE = 0x1 << 2, // 4
  };

  DataChannel(webrtc::DataChannelInterface* dci);
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
  static Handle<Value> New( const Arguments& args );

  static Handle<Value> Send( const Arguments& args );
  static Handle<Value> Close( const Arguments& args );

  static Handle<Value> GetLabel( Local<String> property, const AccessorInfo& info );
  //static Handle<Value> GetBinaryType( Local<String> property, const AccessorInfo& info );
  static Handle<Value> GetReadyState( Local<String> property, const AccessorInfo& info );
  static void ReadOnly( Local<String> property, Local<Value> value, const AccessorInfo& info );

  void QueueEvent(DataChannel::AsyncEventType type, void* data);

private:
  static void Run(uv_async_t* handle, int status);

  struct AsyncEvent {
    AsyncEventType type;
    void* data;
  };

  uv_mutex_t lock;
  uv_async_t async;
  std::queue<AsyncEvent> _events;

  talk_base::scoped_refptr<webrtc::DataChannelInterface> _internalDataChannel;
};

#endif