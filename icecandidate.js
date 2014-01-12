function RTCIceCandidate(dict) {
  this.candidate = {
    'candidate': dict.candidate,
    'sdpMid': dict.sdpMid,
    'sdpMLineIndex': dict.hasOwnProperty('sdpMLineIndex') ? dict.sdpMLineIndex : 0
  };
}

module.exports = RTCIceCandidate;