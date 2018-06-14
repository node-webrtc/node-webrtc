#ifndef __FILEAUDIODEVICEFACTORY_H__
#define __FILEAUDIODEVICEFACTORY_H__

#include <string>

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <uv.h>

#include "webrtc/base/scoped_ref_ptr.h"
#include "webrtc/api/peerconnectioninterface.h"
#include "webrtc/api/mediastreaminterface.h"
#include "peerconnectionfactory.h"

#include "common.h"
#include "nan.h"

using namespace node;
using namespace v8;

namespace node_webrtc {

class FileAudioDeviceFactory
: public Nan::ObjectWrap {

public:
  // Class implementation
  explicit FileAudioDeviceFactory() {};
  ~FileAudioDeviceFactory() {};

  // NodeJS Wrapping
  // NodeJS Wrapping
  static void Init(v8::Handle<v8::Object> exports);
  static Nan::Persistent<Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(SetFilenamesToUse);
};

} // namespace node_webrtc

#endif
