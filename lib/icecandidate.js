'use strict';

function RTCIceCandidate(candidateInitDict) {
  [
    'candidate',
    'sdpMid',
    'sdpMLineIndex',
    'foundation',
    'component',
    'priority',
    'address',
    'protocol',
    'port',
    'type',
    'tcpType',
    'relatedAddress',
    'relatedPort',
    'usernameFragment'
  ].forEach(property => {
    if (candidateInitDict && property in candidateInitDict) {
      this[property] = candidateInitDict[property];
    } else {
      this[property] = null;
    }
  });

  this.toJSON = () => {
    const { candidate, sdpMid, sdpMLineIndex, usernameFragment } = this;
    let json = {
      candidate,
      sdpMid,
      sdpMLineIndex
    };

    if (usernameFragment) {
      json.usernameFragment = usernameFragment;
    }

    return json;
  };
}

module.exports = RTCIceCandidate;
