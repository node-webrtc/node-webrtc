#include "src/dictionaries/webrtc/ice_server.h"

#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

#include <nan.h>
#include <node-addon-api/napi.h>
#include <v8.h>
#include <webrtc/api/peer_connection_interface.h>

#include "src/converters.h"
#include "src/converters/napi.h"
#include "src/converters/v8.h"
#include "src/dictionaries/macros/napi.h"
#include "src/dictionaries/node_webrtc/rtc_outh_credential.h"
#include "src/enums/node_webrtc/rtc_ice_credential_type.h"
#include "src/functional/either.h"
#include "src/functional/validation.h"

namespace node_webrtc {

using stringOrCredential = Either<std::string, RTCOAuthCredential>;
using stringOrStrings = Either<std::vector<std::string>, std::string>;

#define ICE_SERVER_FN CreateIceServer
#define ICE_SERVER_LIST \
  DICT_REQUIRED(stringOrStrings, urls, "urls") \
  DICT_DEFAULT(std::string, username, "username", "") \
  DICT_DEFAULT(stringOrCredential, credential, "credential", MakeLeft<RTCOAuthCredential>(std::string(""))) \
  DICT_DEFAULT(RTCIceCredentialType, credentialType, "credentialType", RTCIceCredentialType::kPassword)


static Validation<webrtc::PeerConnectionInterface::IceServer> ICE_SERVER_FN(
    const Either<std::vector<std::string>, std::string>& urlsOrUrl,
    const std::string& username,
    const Either<std::string, RTCOAuthCredential>& credential,
    const RTCIceCredentialType credentialType) {
  if (credential.IsRight() || credentialType != RTCIceCredentialType::kPassword) {
    return Validation<webrtc::PeerConnectionInterface::IceServer>::Invalid("OAuth is not currently supported");
  }
  webrtc::PeerConnectionInterface::IceServer iceServer;
  iceServer.urls = urlsOrUrl.FromLeft(std::vector<std::string>());
  iceServer.uri = urlsOrUrl.FromRight("");
  iceServer.username = username;
  iceServer.password = credential.UnsafeFromLeft();
  return Pure(iceServer);
}

TO_JS_IMPL(webrtc::PeerConnectionInterface::IceServer, iceServer) {
  Nan::EscapableHandleScope scope;
  auto object = Nan::New<v8::Object>();
  if (!iceServer.uri.empty()) {
    object->Set(Nan::New("urls").ToLocalChecked(), Nan::New(iceServer.uri).ToLocalChecked());
  } else {
    auto maybeArray = From<v8::Local<v8::Value>>(iceServer.urls);
    if (maybeArray.IsInvalid()) {
      return Validation<v8::Local<v8::Value>>::Invalid(maybeArray.ToErrors());
    }
    object->Set(Nan::New("urls").ToLocalChecked(), maybeArray.UnsafeFromValid());
  }
  if (!iceServer.username.empty()) {
    object->Set(Nan::New("username").ToLocalChecked(), Nan::New(iceServer.username).ToLocalChecked());
  }
  if (!iceServer.password.empty()) {
    object->Set(Nan::New("credential").ToLocalChecked(), Nan::New(iceServer.password).ToLocalChecked());
    object->Set(Nan::New("credentialType").ToLocalChecked(), Nan::New("password").ToLocalChecked());
  }
  return Pure(scope.Escape(object.As<v8::Value>()));
}

TO_NAPI_IMPL(webrtc::PeerConnectionInterface::IceServer, pair) {
  auto env = pair.first;
  Napi::EscapableHandleScope scope(env);

  NODE_WEBRTC_CREATE_OBJECT_OR_RETURN(env, object)

  auto iceServer = pair.second;
  if (!iceServer.uri.empty()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "urls", iceServer.uri)
  } else {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "urls", iceServer.urls)
  }
  if (!iceServer.username.empty()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "username", iceServer)
  }
  if (!iceServer.password.empty()) {
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "credential", iceServer.password)
    NODE_WEBRTC_CONVERT_AND_SET_OR_RETURN(env, object, "credentialType", RTCIceCredentialType::kPassword)
  }

  return Pure(scope.Escape(object));
}

}  // namespace node_webrtc

#define DICT(X) ICE_SERVER ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
