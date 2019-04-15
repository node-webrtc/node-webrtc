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
#include "src/converters/napi.h"

TEST_CASE("converting booleans", "[converting-booleans]") {
  auto env = *node_webrtc::Test::env;

  SECTION("from JavaScript") {  // NOLINT
    SECTION("works for") {
      SECTION("true") {
        Napi::Value True = Napi::Boolean::New(env, true);
        REQUIRE(node_webrtc::From<bool>(True) == node_webrtc::Pure(true));
      }

      SECTION("false") {
        Napi::Value False = Napi::Boolean::New(env, false);
        REQUIRE(node_webrtc::From<bool>(False) == node_webrtc::Pure(false));
      }

      // NOTE(mroberts): Not sure if we should fail or not.
      SECTION("null") {
        Napi::Value Null = env.Null();
        REQUIRE(node_webrtc::From<bool>(Null) == node_webrtc::Pure(false));
      }

      // NOTE(mroberts): Not sure if we should fail or not.
      SECTION("undefined") {
        Napi::Value Undefined = env.Undefined();
        REQUIRE(node_webrtc::From<bool>(Undefined) == node_webrtc::Pure(false));
      }
    }
  }
}

TEST_CASE("converting objects", "[converting-objects]") {
  auto env = *node_webrtc::Test::env;

  SECTION("from JavaScript") {
    SECTION("works for") {
      SECTION("objects") {
        Napi::Object object = Napi::Object::New(env);
        REQUIRE(node_webrtc::From<Napi::Object>(object.As<Napi::Value>()) == node_webrtc::Pure(object));
      }
    }

    SECTION("fails for") {
      SECTION("null") {
        Napi::Value Null = env.Null();
        REQUIRE(node_webrtc::From<Napi::Object>(Null).IsInvalid());
      }

      SECTION("undefined") {
        Napi::Value Undefined = env.Undefined();
        REQUIRE(node_webrtc::From<Napi::Object>(Undefined).IsInvalid());
      }
    }
  }
}

TEST_CASE("converting arrays", "[converting-arrays]") {
  auto env = *node_webrtc::Test::env;

  SECTION("from JavaScript") {
    SECTION("works for") {
      SECTION("empty arrays") {
        Napi::Array array = Napi::Array::New(env);
        Napi::Value value = array;
        REQUIRE(node_webrtc::From<Napi::Array>(value) == node_webrtc::Pure(array));
      }

      SECTION("arrays of objects") {
        Napi::Object object1 = Napi::Object::New(env);
        Napi::Object object2 = Napi::Object::New(env);
        std::vector<Napi::Object> expected = {object1, object2};

        Napi::Array array = Napi::Array::New(env);
        array.Set(static_cast<uint32_t>(0), object1);
        array.Set(1, object2);

        REQUIRE(node_webrtc::From<std::vector<Napi::Object>>(array.As<Napi::Value>()) == node_webrtc::Pure(expected));
      }
    }

    SECTION("fails for") {
      SECTION("null") {
        Napi::Value Null = env.Null();
        REQUIRE(node_webrtc::From<Napi::Array>(Null).IsInvalid());
      }

      SECTION("undefined") {
        Napi::Value Undefined = env.Undefined();
        REQUIRE(node_webrtc::From<Napi::Array>(Undefined).IsInvalid());
      }
    }
  }
}

Napi::Env* node_webrtc::Test::env = nullptr;

Napi::Value node_webrtc::Test::TestImpl(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  Test::env = &env;
  auto result = Catch::Session().run();
  CONVERT_OR_THROW_AND_RETURN_NAPI(info.Env(), result, value, Napi::Value)
  return value;
}

void node_webrtc::Test::Init(Napi::Env env, Napi::Object exports) {
  auto func = Napi::Function::New(env, TestImpl);
  exports.Set("test", func);
}

#endif  // DEBUG
