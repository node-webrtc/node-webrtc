/* Copyright (c) 2018 The node-webrtc project authors. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be found
 * in the LICENSE.md file in the root of the source tree. All contributing
 * project authors may be found in the AUTHORS file in the root of the source
 * tree.
 */
#ifndef SRC_ERROR_H_
#define SRC_ERROR_H_

// https://stackoverflow.com/a/13842784
template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };

// https://stackoverflow.com/a/17624752
#define NODE_WEBRTC_PP_CAT(a, b) NODE_WEBRTC_PP_CAT_I(a, b)
#define NODE_WEBRTC_PP_CAT_I(a, b) NODE_WEBRTC_PP_CAT_II(~, a ## b)
#define NODE_WEBRTC_PP_CAT_II(p, res) res
#define NODE_WEBRTC_UNIQUE_NAME(base) NODE_WEBRTC_PP_CAT(base, __LINE__)

// https://stackoverflow.com/a/41566342
#ifndef COMMA
#define COMMA ,
#endif  // COMMA

#define CONVERT_ARGS_OR_THROW_AND_RETURN(O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::Validation<argument_type<void(T)>::type>::Invalid(std::vector<node_webrtc::Error>()); \
  { \
    Nan::TryCatch tc; \
    NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::From<argument_type<void(T)>::type>(node_webrtc::Arguments(info)); \
    if (tc.HasCaught()) { \
      tc.ReThrow(); \
      return; \
    } \
  } \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked()); \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

#define CONVERT_OR_THROW_AND_RETURN(I, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::Validation<argument_type<void(T)>::type>::Invalid(std::vector<node_webrtc::Error>()); \
  { \
    Nan::TryCatch tc; \
    NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::From<argument_type<void(T)>::type>(I); \
    if (tc.HasCaught()) { \
      tc.ReThrow(); \
      return; \
    } \
  } \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked()); \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

#define CONVERT_ARGS_OR_REJECT_AND_RETURN(R, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::Validation<argument_type<void(T)>::type>::Invalid(std::vector<node_webrtc::Error>()); \
  { \
    Nan::TryCatch tc; \
    NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::From<argument_type<void(T)>::type>(node_webrtc::Arguments(info)); \
    if (tc.HasCaught()) { \
      resolver->Resolve(tc.Exception()); \
      return; \
    } \
  } \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    resolver->Resolve(Nan::TypeError(Nan::New(error).ToLocalChecked())); \
    return; \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

#define CONVERT_OR_REJECT_AND_RETURN(R, I, O, T) \
  auto NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::Validation<argument_type<void(T)>::type>::Invalid(std::vector<node_webrtc::Error>()); \
  { \
    Nan::TryCatch tc; \
    NODE_WEBRTC_UNIQUE_NAME(validation) = node_webrtc::From<argument_type<void(T)>::type>(I); \
    if (tc.HasCaught()) { \
      resolver->Resolve(tc.Exception()); \
      return; \
    } \
  } \
  if (NODE_WEBRTC_UNIQUE_NAME(validation).IsInvalid()) { \
    auto error = NODE_WEBRTC_UNIQUE_NAME(validation).ToErrors()[0]; \
    resolver->Resolve(Nan::TypeError(Nan::New(error).ToLocalChecked())); \
    return; \
  } \
  auto O = NODE_WEBRTC_UNIQUE_NAME(validation).UnsafeFromValid();

#define SETUP_PROMISE(...) \
  auto NODE_WEBRTC_UNIQUE_NAME(pair) = PromiseEvent<__VA_ARGS__>::Create(); \
  auto resolver = NODE_WEBRTC_UNIQUE_NAME(pair).first; \
  auto promise = std::move(NODE_WEBRTC_UNIQUE_NAME(pair).second); \
  info.GetReturnValue().Set(resolver->GetPromise());

#endif  // SRC_ERROR_H_
