#ifndef SRC_SET_LOCAL_DESCRIPTION_OBSERVER_H_
#define SRC_SET_LOCAL_DESCRIPTION_OBSERVER_H_

#include <string>

#include "webrtc/api/jsep.h"

namespace node_webrtc {

class PeerConnection;

class SetLocalDescriptionObserver
:  public webrtc::SetSessionDescriptionObserver {
 private:
  PeerConnection* parent;

 public:
  explicit SetLocalDescriptionObserver(PeerConnection* connection): parent(connection) {}

  virtual void OnSuccess();
  virtual void OnFailure(const std::string& msg);
};

}  // namespace node_webrtc

#endif  // SRC_SET_LOCAL_DESCRIPTION_OBSERVER_H_
