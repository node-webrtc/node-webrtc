/* Copyright (c) 2019 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifdef DEBUG

#include "src/test.h"

#define CATCH_CONFIG_RUNNER
#include <catch2/catch.hpp>

#include "src/converters.h"
#include "src/converters/v8.h"

TEST_CASE("converting booleans", "[converting-booleans]") {
  SECTION("from JavaScript") {
    SECTION("works for") {
      SECTION("true") {
        v8::Local<v8::Value> True = Nan::True();
        REQUIRE(node_webrtc::From<bool>(True) == node_webrtc::Pure(true));
      }

      SECTION("false") {
        v8::Local<v8::Value> False = Nan::False();
        REQUIRE(node_webrtc::From<bool>(False) == node_webrtc::Pure(false));
      }

      // NOTE(mroberts): Not sure if we should fail or not.
      SECTION("null") {
        v8::Local<v8::Value> Null = Nan::Null();
        REQUIRE(node_webrtc::From<bool>(Null) == node_webrtc::Pure(false));
      }

      // NOTE(mroberts): Not sure if we should fail or not.
      SECTION("undefined") {
        v8::Local<v8::Value> Undefined = Nan::Undefined();
        REQUIRE(node_webrtc::From<bool>(Undefined) == node_webrtc::Pure(false));
      }
    }
  }
}

TEST_CASE("converting objects", "[converting-objects]") {
  SECTION("from JavaScript") {
    SECTION("works for") {
      SECTION("objects") {
        v8::Local<v8::Object> object = Nan::New<v8::Object>();
        REQUIRE(node_webrtc::From<v8::Local<v8::Object>>(object.As<v8::Value>()) == node_webrtc::Pure(object));
      }
    }

    SECTION("fails for") {
      SECTION("null") {
        v8::Local<v8::Value> Null = Nan::Null();
        REQUIRE(node_webrtc::From<v8::Local<v8::Object>>(Null).IsInvalid());
      }

      SECTION("undefined") {
        v8::Local<v8::Value> Undefined = Nan::Undefined();
        REQUIRE(node_webrtc::From<v8::Local<v8::Object>>(Undefined).IsInvalid());
      }
    }
  }
}

TEST_CASE("converting arrays", "[converting-arrays]") {
  SECTION("from JavaScript") {
    SECTION("works for") {
      SECTION("empty arrays") {
        v8::Local<v8::Array> array = Nan::New<v8::Array>();
        v8::Local<v8::Value> value = array;
        REQUIRE(node_webrtc::From<v8::Local<v8::Array>>(value) == node_webrtc::Pure(array));
      }

      SECTION("arrays of objects") {
        v8::Local<v8::Object> object1 = Nan::New<v8::Object>();
        v8::Local<v8::Object> object2 = Nan::New<v8::Object>();
        std::vector<v8::Local<v8::Object>> expected = {object1, object2};

        v8::Local<v8::Array> array = Nan::New<v8::Array>();
        array->Set(0, object1);
        array->Set(1, object2);

        REQUIRE(node_webrtc::From<std::vector<v8::Local<v8::Object>>>(array.As<v8::Value>()) == node_webrtc::Pure(expected));
      }
    }

    SECTION("fails for") {
      SECTION("null") {
        v8::Local<v8::Value> Null = Nan::Null();
        REQUIRE(node_webrtc::From<v8::Local<v8::Array>>(Null).IsInvalid());
      }

      SECTION("undefined") {
        v8::Local<v8::Value> Undefined = Nan::Undefined();
        REQUIRE(node_webrtc::From<v8::Local<v8::Array>>(Undefined).IsInvalid());
      }
    }
  }
}

NAN_METHOD(node_webrtc::Test::TestImpl) {
  auto result = Catch::Session().run();
  info.GetReturnValue().Set(result);
}

void node_webrtc::Test::Init(v8::Handle<v8::Object> exports) {
  Nan::SetMethod(exports, "test", TestImpl);
}

#endif  // DEBUG
