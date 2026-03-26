#pragma once

#include <string>

#include "src/converters.hh"
#include "src/converters/napi.hh"
#include "src/functional/either.hh"
#include "src/node/error_factory.hh"

namespace webrtc {
class RTCError;
}

namespace node_webrtc {

class SomeError {
public:
  explicit SomeError(const std::string &message)
      : SomeError(message, MakeRight<ErrorFactory::DOMExceptionName>(
                               ErrorFactory::kError)) {}

  SomeError(
      std::string message,
      const Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>
          name)
      : _message(std::move(message)), _name(name) {}

  [[nodiscard]] std::string message() const { return _message; }

  [[nodiscard]] Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName>
  name() const {
    return _name;
  }

private:
  std::string _message;
  Either<ErrorFactory::DOMExceptionName, ErrorFactory::ErrorName> _name;
};

DECLARE_CONVERTER(webrtc::RTCError *, SomeError)
DECLARE_CONVERTER(const webrtc::RTCError *, SomeError)
DECLARE_TO_NAPI(SomeError)

} // namespace node_webrtc
