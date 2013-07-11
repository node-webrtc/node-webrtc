#include <node.h>
#include <v8.h>

#include "common.h"

using namespace v8;

class Test
  : public node::ObjectWrap
{
public:
  static void Init(v8::Handle<v8::Object> exports);

private:
  Test();
  ~Test();
  static v8::Handle<v8::Value> New(const v8::Arguments& args);
  static v8::Handle<v8::Value> RunCallback(const v8::Arguments& args);
};

void Test::Init(v8::Handle<v8::Object> exports) {
  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(New);
  tpl->SetClassName(String::NewSymbol("Test"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototype
  tpl->PrototypeTemplate()->Set(String::NewSymbol("oncomplete"),
    Null());
  tpl->PrototypeTemplate()->Set(String::NewSymbol("RunCallback"),
    FunctionTemplate::New(RunCallback)->GetFunction());

  Persistent<Function> constructor = Persistent<Function>::New(tpl->GetFunction());
  exports->Set(String::NewSymbol("Test"), constructor);
}

Handle<Value> Test::New(const Arguments& args) {
  HandleScope scope;

  Test* obj = new Test();
  obj->Wrap(args.This());

  return args.This();
}

Test::Test() {}
Test::~Test() {}

v8::Handle<v8::Value> Test::RunCallback(const v8::Arguments& args) {
  HandleScope scope;

  if(args.This()->Get(String::New("oncomplete"))->IsNull()) {
    INFO("oncomplete is null");
  } else {
    INFO("oncomplete is not null");
    v8::Local<v8::Function> callback = v8::Local<v8::Function>::Cast(args.This()->Get(String::New("oncomplete")));
    v8::Local<v8::Value> argv[0];
    callback->Call(args.This(), 0, argv);
  }

  return scope.Close( Undefined() );
}

void InitAll(Handle<Object> exports) {
  Test::Init(exports);
}

NODE_MODULE(addon, InitAll)