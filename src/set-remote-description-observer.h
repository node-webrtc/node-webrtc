#ifndef SRC_SET_REMOTE_DESCRIPTION_OBSERVER_H_
#define SRC_SET_REMOTE_DESCRIPTION_OBSERVER_H_

#include <string>

#include "webrtc/api/jsep.h"

namespace node_webrtc {

class PeerConnection;

class SetRemoteDescriptionObserver
: public webrtc::SetSessionDescriptionObserver {
 private:
  PeerConnection* parent;

 public:
  explicit SetRemoteDescriptionObserver(PeerConnection* connection): parent(connection) {}

  virtual void OnSuccess();
  virtual void OnFailure(const std::string& msg);
};

}  // namespace node_webrtc

#endif  // SRC_SET_REMOTE_DESCRIPTION_OBSERVER_H_
