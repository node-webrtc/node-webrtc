#ifdef DEBUG
#define CATCH_CONFIG_RUNNER
#include "test.h"

#include <nan.h>

#include "catch.hpp"
#include "webrtc/api/peerconnectioninterface.h"

#include "src/functional/maybe.h"
#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/webrtc.h"
#include "src/converters/v8.h"

using node_webrtc::From;
using node_webrtc::Maybe;
using node_webrtc::Test;
using v8::Context;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::JSON;
using v8::Local;
using v8::Object;
using v8::Value;
using webrtc::SessionDescriptionInterface;

TEST_CASE("String") {
  SECTION("empty string") {
    std::string emptyString;
    Local<Value> emptyString_ = Nan::New(emptyString).ToLocalChecked();
    REQUIRE(From<std::string>(emptyString_).UnsafeFromValid() == emptyString);
  }

  SECTION("short string") {
    std::string shortString = "short string";
    Local<Value> shortString_ = Nan::New(shortString).ToLocalChecked();
    REQUIRE(From<std::string>(shortString_).UnsafeFromValid() == shortString);
  }

  SECTION("long string") {
    std::string longString;
    for (uint32_t i = 0; i < sizeof(uint32_t); i++) {
      longString += "a";
    }
    Local<Value> longString_ = Nan::New(longString).ToLocalChecked();
    REQUIRE(From<std::string>(longString_).UnsafeFromValid() == longString);
  }
}

TEST_CASE("RTCSessionDescriptionInit") {
  Local<Context> context;
  Nan::HandleScope scope;

  SECTION("offer") {
    auto sdp = "v=0\r\n"
        "o=- 0 1 IN IP4 127.0.0.1\r\n"
        "s=-\r\n"
        "t=0 0\r\n"
        "a=msid-semantic: WMS stream\r\n"
        "m=audio 9 UDP/TLS/RTP/SAVPF 0\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "a=rtcp:9 IN IP4 0.0.0.0\r\n"
        "a=ice-ufrag:0000\r\n"
        "a=ice-pwd:0000000000000000000000\r\n"
        "a=fingerprint:sha-256 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00\r\n"
        "a=mid:audio\r\n"
        "a=sendonly\r\n"
        "a=rtcp-mux\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=ssrc:1 cname:0\r\n"
        "a=ssrc:1 msid:stream track1\r\n"
        "a=ssrc:1 mslabel:stream\r\n"
        "a=ssrc:1 label:track1\r\n";
    Local<Object> offer = Nan::New<Object>();
    offer->Set(Nan::New("type").ToLocalChecked(), Nan::New("offer").ToLocalChecked());
    offer->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
    auto maybeOffer = From<SessionDescriptionInterface*>(static_cast<Local<Value>>(offer));
    if (maybeOffer.IsInvalid()) {
      throw maybeOffer.ToErrors()[0];
    }
    auto offer_ = maybeOffer.UnsafeFromValid();
    REQUIRE(offer_->type() == "offer");
    std::string sdp_;
    offer_->ToString(&sdp_);
    REQUIRE(sdp_ == sdp);
    delete offer_;
  }

  SECTION("answer") {
    auto sdp = "v=0\r\n"
        "o=- 0 1 IN IP4 127.0.0.1\r\n"
        "s=-\r\n"
        "t=0 0\r\n"
        "a=msid-semantic: WMS stream\r\n"
        "m=audio 9 UDP/TLS/RTP/SAVPF 0\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "a=rtcp:9 IN IP4 0.0.0.0\r\n"
        "a=ice-ufrag:0000\r\n"
        "a=ice-pwd:0000000000000000000000\r\n"
        "a=fingerprint:sha-256 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00\r\n"
        "a=mid:audio\r\n"
        "a=sendonly\r\n"
        "a=rtcp-mux\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=ssrc:2 cname:0\r\n"
        "a=ssrc:2 msid:stream track2\r\n"
        "a=ssrc:2 mslabel:stream\r\n"
        "a=ssrc:2 label:track2\r\n";
    Local<Object> answer = Nan::New<Object>();
    answer->Set(Nan::New("type").ToLocalChecked(), Nan::New("answer").ToLocalChecked());
    answer->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
    auto maybeAnswer = From<SessionDescriptionInterface*>(static_cast<Local<Value>>(answer));
    if (maybeAnswer.IsInvalid()) {
      throw maybeAnswer.ToErrors()[0];
    }
    auto answer_ = maybeAnswer.UnsafeFromValid();
    REQUIRE(answer_->type() == "answer");
    std::string sdp_;
    answer_->ToString(&sdp_);
    REQUIRE(sdp_ == sdp);
    delete answer_;
  }

  SECTION("pranswer") {
    auto sdp = "v=0\r\n"
        "o=- 0 1 IN IP4 127.0.0.1\r\n"
        "s=-\r\n"
        "t=0 0\r\n"
        "a=msid-semantic: WMS stream\r\n"
        "m=audio 9 UDP/TLS/RTP/SAVPF 0\r\n"
        "c=IN IP4 0.0.0.0\r\n"
        "a=rtcp:9 IN IP4 0.0.0.0\r\n"
        "a=ice-ufrag:0000\r\n"
        "a=ice-pwd:0000000000000000000000\r\n"
        "a=fingerprint:sha-256 00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00\r\n"
        "a=mid:audio\r\n"
        "a=sendonly\r\n"
        "a=rtcp-mux\r\n"
        "a=rtpmap:0 PCMU/8000\r\n"
        "a=ssrc:2 cname:0\r\n"
        "a=ssrc:2 msid:stream track2\r\n"
        "a=ssrc:2 mslabel:stream\r\n"
        "a=ssrc:2 label:track2\r\n";
    Local<Object> prAnswer = Nan::New<Object>();
    prAnswer->Set(Nan::New("type").ToLocalChecked(), Nan::New("pranswer").ToLocalChecked());
    prAnswer->Set(Nan::New("sdp").ToLocalChecked(), Nan::New(sdp).ToLocalChecked());
    auto maybePrAnswer = From<SessionDescriptionInterface*>(static_cast<Local<Value>>(prAnswer));
    if (maybePrAnswer.IsInvalid()) {
      throw maybePrAnswer.ToErrors()[0];
    }
    auto prAnswer_ = maybePrAnswer.UnsafeFromValid();
    REQUIRE(prAnswer_->type() == "pranswer");
    std::string sdp_;
    prAnswer_->ToString(&sdp_);
    REQUIRE(sdp_ == sdp);
    delete prAnswer_;
  }

  SECTION("rollback") {
    Local<Object> rollback = Nan::New<Object>();
    rollback->Set(Nan::New("type").ToLocalChecked(), Nan::New("rollback").ToLocalChecked());
    auto maybeRollback = From<SessionDescriptionInterface*>(static_cast<Local<Value>>(rollback));
    auto error = maybeRollback.ToErrors()[0];
    REQUIRE(error == "Rollback is not currently supported");
  }
}

static bool alreadyRan = false;
static int result = 0;

static int test(const std::vector<std::string>& args) {
  if (alreadyRan) {
    return result;
  }
  alreadyRan = true;

  auto args_ = std::vector<const char *>();
  for (const std::string& arg : args) {
    args_.push_back(arg.c_str());
  }

  result = Catch::Session().run(static_cast<int>(args_.size()), args_.data());
  result = (result < 0xff ? result : 0xff);

  return result;
}

NAN_METHOD(Main) {
  auto args = From<Maybe<std::vector<std::string>>, Nan::NAN_METHOD_ARGS_TYPE>(info).Map(
      [](const Maybe<std::vector<std::string>> maybeArgs) { return maybeArgs.FromMaybe(std::vector<std::string>()); });
  if (args.IsInvalid()) {
    auto error = args.ToErrors()[0];
    return Nan::ThrowTypeError(Nan::New(error).ToLocalChecked());
  }
  info.GetReturnValue().Set(Nan::New(test(args.UnsafeFromValid())));
}

void Test::Init(Handle<Object> exports) {
  exports->Set(Nan::New("test").ToLocalChecked(), Nan::New<FunctionTemplate>(Main)->GetFunction());
}

#endif  // DEBUG
