#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "videomediachannel.h"

using namespace node;
using namespace v8;

Persistent<Function> VideoMediaChannel::constructor;

VideoMediaChannel::VideoMediaChannel(cricket::VideoMediaChannel* vmc)
: _internalVideoMediaChannel(vmc)
{

}

VideoMediaChannel::~VideoMediaChannel()
{
    
}

NAN_METHOD(VideoMediaChannel::New) {
  TRACE_CALL;
  NanScope();

  if(!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the VideoMediaChannel.");
  }

  v8::Local<v8::External> _vmc = v8::Local<v8::External>::Cast(args[0]);
  cricket::VideoMediaChannel* vmc = static_cast<cricket::VideoMediaChannel*>(_vmc->Value());

  VideoMediaChannel* obj = new VideoMediaChannel(vmc);
  obj->Wrap( args.This() );

  TRACE_END;
  NanReturnValue( args.This() );
}

NAN_METHOD(VideoMediaChannel::SetRenderer) {
  TRACE_CALL;
  NanScope();

  VideoMediaChannel* self = ObjectWrap::Unwrap<VideoMediaChannel>( args.This() );
  v8::Local<v8::External> _vr = v8::Local<v8::External>::Cast(args[0]);

  cricket::VideoRenderer* vr = static_cast<cricket::VideoRenderer*>(_vr->Value());
  self->_internalVideoMediaChannel->SetRenderer(0, vr);
  
  TRACE_END;
  NanReturnValue(Boolean::New(true));
}

void VideoMediaChannel::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "VideoMediaChannel" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(0);
    
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setRenderer" ),
    FunctionTemplate::New( SetRenderer )->GetFunction() );

  NanAssignPersistent(Function, constructor, tpl->GetFunction());
  exports->Set( String::NewSymbol("VideoMediaChannel"), tpl->GetFunction() );
}
/* ex: set tabstop=2 expandtab: */
