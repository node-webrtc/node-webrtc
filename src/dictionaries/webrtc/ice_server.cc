#include "src/dictionaries/webrtc/ice_server.h"

#include <iosfwd>
#include <string>
#include <vector>

#include <webrtc/api/peer_connection_interface.h>

#include "src/dictionaries/node_webrtc/rtc_outh_credential.h"
#include "src/enums/node_webrtc/rtc_ice_credential_type.h"
#include "src/functional/either.h"
#include "src/functional/validation.h"

namespace node_webrtc {

typedef Either<std::string, RTCOAuthCredential> stringOrCredential;
typedef Either<std::vector<std::string>, std::string> stringOrStrings;

#define ICE_SERVER_FN CreateIceServer
#define ICE_SERVER_LIST \
  REQUIRED(stringOrStrings, urls, "urls") \
  DEFAULT(std::string, username, "username", "") \
  DEFAULT(stringOrCredential, credential, "credential", MakeLeft<RTCOAuthCredential>(std::string(""))) \
  DEFAULT(RTCIceCredentialType, credentialType, "credentialType", RTCIceCredentialType::kPassword)


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

}  // namespace node_webrtc

#define DICT(X) ICE_SERVER ## X
#include "src/dictionaries/macros/impls.h"
#undef DICT
