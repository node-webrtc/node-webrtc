#ifdef DEBUG
#define CATCH_CONFIG_RUNNER
#include "test.h"

#include "catch.hpp"
#include "nan.h"
#include "webrtc/api/peerconnectioninterface.h"

#include "src/functional/maybe.h"
#include "src/converters.h"
#include "src/converters/arguments.h"
#include "src/converters/webrtc.h"
#include "src/converters/v8.h"

using node_webrtc::From;
using node_webrtc::Maybe;
using node_webrtc::RTCDtlsFingerprint;
using node_webrtc::RTCIceCredentialType;
using node_webrtc::RTCOAuthCredential;
using node_webrtc::RTCPriorityType;
using node_webrtc::RTCSdpType;
using node_webrtc::Test;
using v8::Function;
using v8::FunctionTemplate;
using v8::Handle;
using v8::JSON;
using v8::Local;
using v8::Object;
using v8::Value;
using webrtc::DataChannelInit;
using webrtc::IceCandidateInterface;
using webrtc::SessionDescriptionInterface;

using IceServer = webrtc::PeerConnectionInterface::IceServer;
using IceTransportsType = webrtc::PeerConnectionInterface::IceTransportsType;
using BundlePolicy = webrtc::PeerConnectionInterface::BundlePolicy;
using RtcpMuxPolicy = webrtc::PeerConnectionInterface::RtcpMuxPolicy;

static Nan::JSON NanJSON;

template <typename T>
static void RequireInvalid(const Local<Value> value) {
  REQUIRE(From<T>(value).IsInvalid());
}

template <typename T>
static T RequireValid(const Local<Value> value) {
  auto maybeValue_ = From<T>(value);
  if (maybeValue_.IsInvalid()) {
    throw std::string(maybeValue_.ToErrors()[0]);
  }
  return maybeValue_.UnsafeFromValid();
}

template <typename T>
static void RequireEnum(const std::string string, const T expected) {
  auto actual = RequireValid<T>(Nan::New(string).ToLocalChecked());
  REQUIRE(actual == expected);
}

TEST_CASE("String") {
  SECTION("valid") {
    SECTION("empty string") {
      std::string expected;
      Local<Value> expected_ = Nan::New(expected).ToLocalChecked();
      REQUIRE(RequireValid<std::string>(expected_) == expected);
    }

    SECTION("short string") {
      std::string expected = "short string";
      Local<Value> expected_ = Nan::New(expected).ToLocalChecked();
      REQUIRE(RequireValid<std::string>(expected_) == expected);
    }

    SECTION("long string") {
      std::string expected;
      for (uint32_t i = 0; i < sizeof(uint32_t); i++) {
        expected += "a";
      }
      Local<Value> expected_ = Nan::New(expected).ToLocalChecked();
      REQUIRE(RequireValid<std::string>(expected_) == expected);
    }
  }

  SECTION("invalid") {
    // TODO(mroberts): Include some invalid strings.
  }
}

TEST_CASE("RTCBundlePolicy") {
  SECTION("valid") {
    RequireEnum("balanced", BundlePolicy::kBundlePolicyBalanced);
    RequireEnum("max-bundle", BundlePolicy::kBundlePolicyMaxBundle);
    RequireEnum("max-compat", BundlePolicy::kBundlePolicyMaxCompat);
  }

  SECTION("invalid") {
    RequireInvalid<BundlePolicy>(Nan::New("").ToLocalChecked());
    RequireInvalid<BundlePolicy>(Nan::New("bogus").ToLocalChecked());
  }
}

TEST_CASE("RTCDataChannelInit") {
  SECTION("valid") {
    Local<Value> emptyObject = Nan::New<Object>();
    auto emptyObject_ = RequireValid<DataChannelInit>(emptyObject);
    REQUIRE(emptyObject_.ordered);
    REQUIRE(emptyObject_.maxRetransmitTime == -1);
    REQUIRE(emptyObject_.maxRetransmits == -1);
    REQUIRE(emptyObject_.protocol == "");
    REQUIRE(!emptyObject_.negotiated);
    REQUIRE(emptyObject_.id == -1);

    Local<Value> fullObject = NanJSON.Parse(Nan::New(R"({
  "ordered": false,
  "maxPacketLifeTime": 666,
  "maxRetransmits": 3,
  "protocol": "foo",
  "negotiated": true,
  "id": 9
})").ToLocalChecked()).ToLocalChecked();
    auto fullObject_ = RequireValid<DataChannelInit>(fullObject);
    REQUIRE(!fullObject_.ordered);
    REQUIRE(fullObject_.maxRetransmitTime == 666);
    REQUIRE(fullObject_.maxRetransmits == 3);
    REQUIRE(fullObject_.protocol == "foo");
    REQUIRE(fullObject_.negotiated);
    REQUIRE(fullObject_.id == 9);
  }

  SECTION("invalid") {
    // TODO(mroberts): Add invalid RTCDataChannelInits.
  }
}

TEST_CASE("RTCDtlsFingerprint") {
  SECTION("valid") {
    Local<Value> emptyObject = Nan::New<Object>();
    auto emptyObject_ = RequireValid<RTCDtlsFingerprint>(emptyObject);
    REQUIRE(emptyObject_.algorithm.IsNothing());
    REQUIRE(emptyObject_.value.IsNothing());

    auto algorithmOnly = NanJSON.Parse(Nan::New(R"({
  "algorithm": "foo"
})").ToLocalChecked()).ToLocalChecked();
    auto algorithmOnly_ = RequireValid<RTCDtlsFingerprint>(algorithmOnly);
    REQUIRE(algorithmOnly_.algorithm.UnsafeFromJust() == "foo");
    REQUIRE(algorithmOnly_.value.IsNothing());

    auto valueOnly = NanJSON.Parse(Nan::New(R"({
  "value": "foo"
})").ToLocalChecked()).ToLocalChecked();
    auto valueOnly_ = RequireValid<RTCDtlsFingerprint>(valueOnly);
    REQUIRE(valueOnly_.algorithm.IsNothing());
    REQUIRE(valueOnly_.value.UnsafeFromJust() == "foo");

    auto fullObject = NanJSON.Parse(Nan::New(R"({
  "algorithm": "foo",
  "value": "bar"
})").ToLocalChecked()).ToLocalChecked();
    auto fullObject_ = RequireValid<RTCDtlsFingerprint>(fullObject);
    REQUIRE(fullObject_.algorithm.UnsafeFromJust() == "foo");
    REQUIRE(fullObject_.value.UnsafeFromJust() == "bar");
  }

  SECTION("invalid") {
    // TODO(mroberts): Add some invalid RTCDtlsFingerprints.
  }
}

TEST_CASE("RTCIceCandidateInit") {
  SECTION("valid") {
    SECTION("no sdpMid or sdpMLineIndex") {
      std::string candidate = "candidate:1467250027 1 udp 2122260223 192.168.0.196 46243 typ host generation 0";
      auto iceCandidate = Nan::New<Object>();
      Nan::Set(iceCandidate, Nan::New("candidate").ToLocalChecked(), Nan::New(candidate).ToLocalChecked());
      auto iceCandidate_ = RequireValid<IceCandidateInterface*>(iceCandidate);
      std::string candidate_;
      iceCandidate_->ToString(&candidate_);
      REQUIRE(candidate_ == candidate);
      REQUIRE(iceCandidate_->sdp_mid() == "");
      REQUIRE(iceCandidate_->sdp_mline_index() == 0);
      delete iceCandidate_;
    }

    SECTION("both sdpMid and sdpMLineIndex") {
      std::string candidate = "candidate:1467250027 1 udp 2122260223 192.168.0.196 46243 typ host generation 0";
      std::string sdpMid = "video";
      int sdpMLineIndex = 1;
      auto iceCandidate = Nan::New<Object>();
      Nan::Set(iceCandidate, Nan::New("candidate").ToLocalChecked(), Nan::New(candidate).ToLocalChecked());
      Nan::Set(iceCandidate, Nan::New("sdpMid").ToLocalChecked(), Nan::New(sdpMid).ToLocalChecked());
      Nan::Set(iceCandidate, Nan::New("sdpMLineIndex").ToLocalChecked(), Nan::New(sdpMLineIndex));
      auto iceCandidate_ = RequireValid<IceCandidateInterface*>(iceCandidate);
      std::string candidate_;
      iceCandidate_->ToString(&candidate_);
      REQUIRE(candidate_ == candidate);
      REQUIRE(iceCandidate_->sdp_mid() == sdpMid);
      REQUIRE(iceCandidate_->sdp_mline_index() == sdpMLineIndex);
      delete iceCandidate_;
    }
  }

  SECTION("invalid") {
    // NOTE(mroberts): Although the IDL says we can default to the empty string,
    // WebRTC wants a valid SDP fragment.
    RequireInvalid<IceCandidateInterface*>(Nan::New<Object>());
  }
}

TEST_CASE("RTCIceCredentialType") {
  SECTION("valid") {
    RequireEnum("oauth", RTCIceCredentialType::kOAuth);
    RequireEnum("password", RTCIceCredentialType::kPassword);
  }

  SECTION("invalid") {
    RequireInvalid<RTCIceCredentialType>(Nan::New("").ToLocalChecked());
    RequireInvalid<RTCIceCredentialType>(Nan::New("bogus").ToLocalChecked());
  }
}

TEST_CASE("RTCIceServer") {
  SECTION("valid") {
    SECTION("url") {
      auto iceServer = NanJSON.Parse(Nan::New(R"({
  "urls": "foo.com"
})").ToLocalChecked()).ToLocalChecked();
      auto actual = RequireValid<IceServer>(iceServer);
      REQUIRE(actual.uri == "foo.com");
    }

    SECTION("urls") {
      auto iceServer = NanJSON.Parse(Nan::New(R"({
  "urls": ["foo.com", "bar.net"]
})").ToLocalChecked()).ToLocalChecked();
      auto actual = RequireValid<IceServer>(iceServer);
      REQUIRE(actual.urls[0] == "foo.com");
      REQUIRE(actual.urls[1] == "bar.net");
    }

    SECTION("username and credential") {
      auto iceServer = NanJSON.Parse(Nan::New(R"({
  "urls": "foo.com",
  "username": "foo",
  "credential": "bar"
})").ToLocalChecked()).ToLocalChecked();
      auto actual = RequireValid<IceServer>(iceServer);
      REQUIRE(actual.username == "foo");
      REQUIRE(actual.password == "bar");
    }
  }

  SECTION("invalid") {
    RequireInvalid<IceServer>(Nan::New<Object>());

    auto oAuth1 = NanJSON.Parse(Nan::New(R"({
  "urls": "foo.com",
  "credentialType": "oauth"
})").ToLocalChecked()).ToLocalChecked();
    auto maybeOAuth1 = From<IceServer>(oAuth1);
    REQUIRE(maybeOAuth1.ToErrors()[0] == "OAuth is not currently supported");

    auto oAuth2 = NanJSON.Parse(Nan::New(R"({
  "urls": "foo.com",
  "credential": {
    "macKey": "foo",
    "accessToken": "bar"
  }
})").ToLocalChecked()).ToLocalChecked();
    auto maybeOAuth2 = From<IceServer>(oAuth2);
    REQUIRE(maybeOAuth2.ToErrors()[0] == "OAuth is not currently supported");
  }
}

TEST_CASE("RTCIceTransportPolicy") {
  SECTION("valid") {
    RequireEnum("all", IceTransportsType::kAll);
    RequireEnum("relay", IceTransportsType::kRelay);
  }

  SECTION("invalid") {
    RequireInvalid<IceTransportsType>(Nan::New("").ToLocalChecked());
    RequireInvalid<IceTransportsType>(Nan::New("bogus").ToLocalChecked());
  }
}

TEST_CASE("RTCOAuthCredential") {
  SECTION("valid") {
    auto rtcOAuthCredential = NanJSON.Parse(Nan::New(R"({
  "macKey": "foo",
  "accessToken": "bar"
})").ToLocalChecked()).ToLocalChecked();
    auto rtcOAuthCredential_ = RequireValid<RTCOAuthCredential>(rtcOAuthCredential);
    REQUIRE(rtcOAuthCredential_.macKey == "foo");
    REQUIRE(rtcOAuthCredential_.accessToken == "bar");
  }

  SECTION("invalid") {
    Local<Value> emptyObject = Nan::New<Object>();
    RequireInvalid<RTCOAuthCredential>(emptyObject);

    auto invalidMacKey = NanJSON.Parse(Nan::New(R"({
  "macKey": {},
  "accessToken": "foo"
})").ToLocalChecked()).ToLocalChecked();
    RequireInvalid<RTCOAuthCredential>(invalidMacKey);

    auto invalidAccessToken = NanJSON.Parse(Nan::New(R"({
  "macKey": "foo",
  "accessToken": {}
})").ToLocalChecked()).ToLocalChecked();
    RequireInvalid<RTCOAuthCredential>(invalidAccessToken);
  }
}

TEST_CASE("RTCPriorityType") {
  SECTION("valid") {
    RequireEnum<RTCPriorityType>("very-low", RTCPriorityType::kVeryLow);
    RequireEnum<RTCPriorityType>("low", RTCPriorityType::kLow);
    RequireEnum<RTCPriorityType>("medium", RTCPriorityType::kMedium);
    RequireEnum<RTCPriorityType>("high", RTCPriorityType::kHigh);
  }

  SECTION("invalid") {
    RequireInvalid<RtcpMuxPolicy>(Nan::New("").ToLocalChecked());
    RequireInvalid<RtcpMuxPolicy>(Nan::New("bogus").ToLocalChecked());
  }
}

TEST_CASE("RTCRtcpMuxPolicy") {
  SECTION("valid") {
    RequireEnum("require", RtcpMuxPolicy::kRtcpMuxPolicyRequire);
    RequireEnum("negotiate", RtcpMuxPolicy::kRtcpMuxPolicyNegotiate);
  }

  SECTION("invalid") {
    RequireInvalid<RtcpMuxPolicy>(Nan::New("").ToLocalChecked());
    RequireInvalid<RtcpMuxPolicy>(Nan::New("bogus").ToLocalChecked());
  }
}

TEST_CASE("RTCSdpType") {
  SECTION("valid") {
    RequireEnum("offer", RTCSdpType::kOffer);
    RequireEnum("answer", RTCSdpType::kAnswer);
    RequireEnum("pranswer", RTCSdpType::kPrAnswer);
    RequireEnum("rollback", RTCSdpType::kRollback);
  }

  SECTION("invalid") {
    RequireInvalid<RTCSdpType>(Nan::New("").ToLocalChecked());
    RequireInvalid<RTCSdpType>(Nan::New("bogus").ToLocalChecked());
  }
}

TEST_CASE("RTCSessionDescriptionInit") {
  SECTION("valid") {
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
      auto offer_ = RequireValid<SessionDescriptionInterface *>(offer);
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
      auto answer_ = RequireValid<SessionDescriptionInterface *>(answer);
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
      auto prAnswer_ = RequireValid<SessionDescriptionInterface *>(prAnswer);
      REQUIRE(prAnswer_->type() == "pranswer");
      std::string sdp_;
      prAnswer_->ToString(&sdp_);
      REQUIRE(sdp_ == sdp);
      delete prAnswer_;
    }
  }

  SECTION("invalid") {
    Local<Object> emptyObject = Nan::New<Object>();
    RequireInvalid<SessionDescriptionInterface*>(emptyObject);

    Local<Object> rollback = Nan::New<Object>();
    rollback->Set(Nan::New("type").ToLocalChecked(), Nan::New("rollback").ToLocalChecked());
    auto maybeRollback = From<SessionDescriptionInterface *>(static_cast<Local<Value>>(rollback));
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
