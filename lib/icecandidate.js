function RTCIceCandidate(dict) {
  this.candidate = dict.candidate;
  this.sdpMid = dict.sdpMid;
  this.sdpMLineIndex = dict.hasOwnProperty('sdpMLineIndex') ? dict.sdpMLineIndex : 0;
}

module.exports = RTCIceCandidate;