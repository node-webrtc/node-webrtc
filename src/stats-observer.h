#ifndef SRC_STATS_OBSERVER_H_
#define SRC_STATS_OBSERVER_H_

#include "nan.h" // IWYU pragma: keep

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/statstypes.h"

namespace node_webrtc {

class PeerConnection;

class StatsObserver
  : public webrtc::StatsObserver
{
  private:
    PeerConnection* parent;
    Nan::Callback* callback;

  public:
    StatsObserver( PeerConnection* parent, Nan::Callback *callback )
    : parent(parent), callback(callback) {};

    virtual void OnComplete(const webrtc::StatsReports& reports);
};

}  // namespace node_webrtc

#endif  // SRC_STATS_OBSERVER_H_
