/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#include "async_object_wrap.h"

#include <node_version.h>

using node_webrtc::AsyncObjectWrap;

AsyncObjectWrap::AsyncObjectWrap(const char* name)  // NOLINT
  : _async_resource(new Nan::AsyncResource(name)) {
  // Do nothing.
}

AsyncObjectWrap::~AsyncObjectWrap() {
  DestroyAsyncResource();
}

void AsyncObjectWrap::AddRef() {
  if (_reference_count.fetch_add(1) == 0) {
    Ref();
  }
}

void AsyncObjectWrap::RemoveRef() {
  if (_reference_count.fetch_sub(1) == 1) {
    Unref();
  }
}

v8::Local<v8::Object> AsyncObjectWrap::ToObject() {
  Nan::EscapableHandleScope scope;
  return scope.Escape(handle());
}

void AsyncObjectWrap::Wrap(v8::Local<v8::Object> object) {
  Nan::ObjectWrap::Wrap(object);
}

void AsyncObjectWrap::MakeCallback(const char* name, const int argc, v8::Local<v8::Value>* argv) {
  _async_resource_mutex.lock();
  if (_async_resource) {
    _async_resource->runInAsyncScope(ToObject(), name, argc, argv);
  }
  _async_resource_mutex.unlock();
}

void AsyncObjectWrap::DestroyAsyncResource() {
  Nan::HandleScope scope;
  _async_resource_mutex.lock();
  if (_async_resource) {
#if NODE_MAJOR_VERSION >= 9
    if (!Nan::GetCurrentContext().IsEmpty()) {
      delete _async_resource;
    } else {
      // HACK(mroberts): This is probably unsafe, but sometimes the current Context is null...
      auto context = v8::Isolate::GetCurrent()->GetEnteredContext();
      context->Enter();
      delete _async_resource;
      context->Exit();
    }
#else
    delete _async_resource;
#endif
    _async_resource = nullptr;
  }
  _async_resource_mutex.unlock();
}
