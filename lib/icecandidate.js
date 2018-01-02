'use strict';

function RTCIceCandidate(candidateInitDict) {
  if (candidateInitDict) {
    this.candidate     = candidateInitDict.candidate;
    this.sdpMid        = candidateInitDict.sdpMid;
    this.sdpMLineIndex = candidateInitDict.sdpMLineIndex;
  }
}

module.exports = RTCIceCandidate;
