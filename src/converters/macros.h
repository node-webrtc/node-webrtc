#pragma once

// https://stackoverflow.com/a/17624752
#define NODE_WEBRTC_PP_CAT(a, b) NODE_WEBRTC_PP_CAT_I(a, b)
#define NODE_WEBRTC_PP_CAT_I(a, b) NODE_WEBRTC_PP_CAT_II(~, a ## b)
#define NODE_WEBRTC_PP_CAT_II(p, res) res
#define NODE_WEBRTC_UNIQUE_NAME(base) NODE_WEBRTC_PP_CAT(base, __LINE__)

// https://stackoverflow.com/a/41566342
#ifndef COMMA
#define COMMA ,
#endif  // COMMA

namespace node_webrtc {
namespace detail {

// https://stackoverflow.com/a/13842784
template<typename T> struct argument_type;
template<typename T, typename U> struct argument_type<T(U)> { typedef U type; };

}  // namespace detail
}  // namespace node_webrtc
