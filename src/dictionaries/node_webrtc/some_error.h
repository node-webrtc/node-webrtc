#pragma once

#include <string>

#include "src/converters.h"
#include "src/converters/napi.h"
#include "src/converters/v8.h"
#include "src/node/error_factory.h"
#include "src/functional/either.h"

namespace webrtc { class RTCError; }

namespace node_webrtc {

class SomeError {
 public:
  SomeError() = default;

  explicit SomeError(
      const std::string& message):
    SomeError(
        message,
        MakeRight<ErrorFactory::DOMExceptionName>(ErrorFactory::kError)) {}

  SomeError(
      const std::string& message,
      const Either<ErrorFactory::DOMExceptionName,
      ErrorFactory::ErrorName> name):
    _message(std::move(message)),
    _name(name) {}

  std::string message() const {
    return _message;
  }

  Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName> name() const {
    return _name;
  }

 private:
  std::string _message;
  Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName> _name;
};

DECLARE_CONVERTER(webrtc::RTCError*, SomeError)
DECLARE_CONVERTER(const webrtc::RTCError*, SomeError)
DECLARE_TO_JS(SomeError)
DECLARE_TO_NAPI(SomeError)

}  // namespace node_webrtc
