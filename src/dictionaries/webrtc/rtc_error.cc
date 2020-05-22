#include "src/dictionaries/webrtc/rtc_error.h"

#include <node-addon-api/napi.h>
#include <utility>
#include <webrtc/api/rtc_error.h>

#include "src/functional/validation.h"
#include "src/node/error_factory.h"

namespace node_webrtc {

TO_NAPI_IMPL(webrtc::RTCError*, pair) {
  return Converter<std::pair<Napi::Env, const webrtc::RTCError*>, Napi::Value>::Convert(pair);
}

TO_NAPI_IMPL(const webrtc::RTCError*, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);
  auto error = pair.second;
  if (!error) {
    return Validation<Napi::Value>::Invalid("No error? Please file a bug on https://github.com/node-webrtc/node-webrtc");
  }
  switch (error->type()) {
    case webrtc::RTCErrorType::NONE:
      return Validation<Napi::Value>::Invalid("No error? Please file a bug on https://github.com/node-webrtc/node-webrtc");
    case webrtc::RTCErrorType::UNSUPPORTED_PARAMETER:
    case webrtc::RTCErrorType::INVALID_PARAMETER:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidAccessError(env, error->message())));
    case webrtc::RTCErrorType::INVALID_RANGE:
      return Pure(scope.Escape(ErrorFactory::CreateRangeError(env, error->message())));
    case webrtc::RTCErrorType::SYNTAX_ERROR:
      return Pure(scope.Escape(ErrorFactory::CreateSyntaxError(env, error->message())));
    case webrtc::RTCErrorType::INVALID_STATE:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidStateError(env, error->message())));
    case webrtc::RTCErrorType::INVALID_MODIFICATION:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidModificationError(env, error->message())));
    case webrtc::RTCErrorType::NETWORK_ERROR:
      return Pure(scope.Escape(ErrorFactory::CreateNetworkError(env, error->message())));
    // NOTE(mroberts): SetLocalDescription in the wrong state can throw this.
    case webrtc::RTCErrorType::INTERNAL_ERROR:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidStateError(env, error->message())));
    case webrtc::RTCErrorType::UNSUPPORTED_OPERATION:
    case webrtc::RTCErrorType::RESOURCE_EXHAUSTED:
      return Pure(scope.Escape(ErrorFactory::CreateOperationError(env, error->message())));
    // NOTE(mroberts): I believe this is supposed to include some additional data.
    case webrtc::RTCErrorType::OPERATION_ERROR_WITH_DATA:
      return Pure(scope.Escape(ErrorFactory::CreateOperationError(env, error->message())));
  }
}

}  // namespace node_webrtc
