exports.RTCIceCandidate       = require('./icecandidate');
exports.RTCPeerConnection     = require('./peerconnection');
exports.RTCSessionDescription = require('./sessiondescription');

var binding = require('./binding');
if (binding.test) {
  exports.test = binding.test;
}
