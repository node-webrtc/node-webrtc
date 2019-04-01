#include "src/dictionaries/webrtc/rtc_error.h"

#include <nan.h>
#include <v8.h>
#include <webrtc/api/rtc_error.h>

#include "src/functional/validation.h"
#include "src/node/error_factory.h"

namespace node_webrtc {

TO_JS_IMPL(webrtc::RTCError*, error) {
  return Converter<const webrtc::RTCError*, v8::Local<v8::Value>>::Convert(error);
}

TO_JS_IMPL(const webrtc::RTCError*, error) {
  Nan::EscapableHandleScope scope;
  if (!error) {
    return Validation<v8::Local<v8::Value>>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
  }
  switch (error->type()) {
    case webrtc::RTCErrorType::NONE:
      return Validation<v8::Local<v8::Value>>::Invalid("No error? Please file a bug on https://github.com/js-platform/node-webrtc");
    case webrtc::RTCErrorType::UNSUPPORTED_PARAMETER:
    case webrtc::RTCErrorType::INVALID_PARAMETER:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidAccessError(error->message())));
    case webrtc::RTCErrorType::INVALID_RANGE:
      return Pure(scope.Escape(ErrorFactory::CreateRangeError(error->message())));
    case webrtc::RTCErrorType::SYNTAX_ERROR:
      return Pure(scope.Escape(ErrorFactory::CreateSyntaxError(error->message())));
    case webrtc::RTCErrorType::INVALID_STATE:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidStateError(error->message())));
    case webrtc::RTCErrorType::INVALID_MODIFICATION:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidModificationError(error->message())));
    case webrtc::RTCErrorType::NETWORK_ERROR:
      return Pure(scope.Escape(ErrorFactory::CreateNetworkError(error->message())));
    // NOTE(mroberts): SetLocalDescription in the wrong state can throw this.
    case webrtc::RTCErrorType::INTERNAL_ERROR:
      return Pure(scope.Escape(ErrorFactory::CreateInvalidStateError(error->message())));
    case webrtc::RTCErrorType::UNSUPPORTED_OPERATION:
    case webrtc::RTCErrorType::RESOURCE_EXHAUSTED:
      return Pure(scope.Escape(ErrorFactory::CreateOperationError(error->message())));
  }
}

}  // namespace node_webrtc
