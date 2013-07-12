#include <v8.h>
#include <node.h>
#include <uv.h>

#include <queue>

using namespace node;
using namespace v8;

class PeerConnection
: public ObjectWrap
{

private:

  uv_mutex_t eventLock;
  uv_async_t emitAsync;;

public:

  PeerConnection();
  ~PeerConnection();

  //
  // Nodejs wrapping.
  //
  static void Init( Handle<Object> exports );
  static Persistent<Function> constructor;
  static Handle<Value> New( const Arguments& args );

  static Handle<Value> CreateOffer( const Arguments& args );
  static Handle<Value> CreateAnswer( const Arguments& args );
  static Handle<Value> SetLocalDescription( const Arguments& args );
  static Handle<Value> SetRemoteDescription( const Arguments& args );
  static Handle<Value> UpdateIce( const Arguments& args );
  static Handle<Value> AddIceCandidate( const Arguments& args );
  static Handle<Value> Close( const Arguments& args );

};
