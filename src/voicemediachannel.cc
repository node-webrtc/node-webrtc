#include <node_buffer.h>

#include <stdint.h>
#include <iostream>
#include <string>

#include "talk/app/webrtc/jsep.h"
#include "webrtc/system_wrappers/interface/ref_count.h"

#include "common.h"
#include "voicemediachannel.h"

using namespace node;
using namespace v8;

Persistent<Function> VoiceMediaChannel::constructor;

VoiceMediaChannel::VoiceMediaChannel(cricket::VoiceMediaChannel* vmc)
: _internalVoiceMediaChannel(vmc)
{

}

VoiceMediaChannel::~VoiceMediaChannel()
{
    
}

NAN_METHOD(VoiceMediaChannel::New) {
  TRACE_CALL;
  NanScope();

  if(!args.IsConstructCall()) {
    return NanThrowTypeError("Use the new operator to construct the VoiceMediaChannel.");
  }

  v8::Local<v8::External> _vmc = v8::Local<v8::External>::Cast(args[0]);
  cricket::VoiceMediaChannel* vmc = static_cast<cricket::VoiceMediaChannel*>(_vmc->Value());

  VoiceMediaChannel* obj = new VoiceMediaChannel(vmc);
  obj->Wrap( args.This() );

  TRACE_END;
  NanReturnValue( args.This() );
}

NAN_METHOD(VoiceMediaChannel::SetLocalRenderer) {
  TRACE_CALL;
  NanScope();

  VoiceMediaChannel* self = ObjectWrap::Unwrap<VoiceMediaChannel>( args.This() );
  v8::Local<v8::External> _vr = v8::Local<v8::External>::Cast(args[0]);

  cricket::AudioRenderer* vr = static_cast<cricket::AudioRenderer*>(_vr->Value());
  self->_internalVoiceMediaChannel->SetLocalRenderer(0, vr);
  
  TRACE_END;
  NanReturnValue(Boolean::New(true));
}

void VoiceMediaChannel::Init( Handle<Object> exports ) {
  Local<FunctionTemplate> tpl = FunctionTemplate::New( New );
  tpl->SetClassName( String::NewSymbol( "VoiceMediaChannel" ) );
  tpl->InstanceTemplate()->SetInternalFieldCount(1);
    
  tpl->PrototypeTemplate()->Set( String::NewSymbol( "setLocalRenderer" ),
    FunctionTemplate::New( SetLocalRenderer )->GetFunction() );

  NanAssignPersistent(Function, constructor, tpl->GetFunction());
  exports->Set( String::NewSymbol("VoiceMediaChannel"), tpl->GetFunction() );
}
/* ex: set tabstop=2 expandtab: */
