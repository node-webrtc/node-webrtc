function RTCIceCandidate(candidateInitDict) {
  'use strict';
  if(candidateInitDict) {
    this.candidate     = candidateInitDict.candidate;
    this.sdpMid        = candidateInitDict.sdpMid;
    this.sdpMLineIndex = candidateInitDict.sdpMLineIndex;
  }
}

module.exports = RTCIceCandidate;
