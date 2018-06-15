#ifndef __FILEAUDIODEVICEFACTORY_H__
#define __FILEAUDIODEVICEFACTORY_H__

#include "common.h"

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
  static Nan::Persistent<v8::Function> constructor;
  static NAN_METHOD(New);

  static NAN_METHOD(SetFilenamesToUse);
};

} // namespace node_webrtc

#endif
