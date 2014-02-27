function RTCIceCandidateEvent(candidate) {
  this.candidate = candidate;
}
RTCIceCandidateEvent.prototype.type = 'icecandidate';


module.exports = RTCIceCandidateEvent;