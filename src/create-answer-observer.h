#ifndef SRC_CREATE_ANSWER_OBSERVER_H_
#define SRC_CREATE_ANSWER_OBSERVER_H_

#include <string>

#include "webrtc/api/jsep.h"

namespace node_webrtc {

class PeerConnection;

class CreateAnswerObserver
: public webrtc::CreateSessionDescriptionObserver {
 private:
  PeerConnection* parent;

 public:
  explicit CreateAnswerObserver(PeerConnection* connection): parent(connection) {}

  virtual void OnSuccess(webrtc::SessionDescriptionInterface* sdp);
  virtual void OnFailure(const std::string& msg);
};

}  // namespace node_webrtc

#endif  // SRC_CREATE_ANSWER_OBSERVER_H_
