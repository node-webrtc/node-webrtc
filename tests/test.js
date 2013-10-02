var webrtc = require('../index');

var pc = new webrtc.RTCPeerConnection();

console.log('\n\n---------------\n\n');

console.log('pc: ' + Object.keys(pc));

console.log('pc.localDescription: ', pc.localDescription);
console.log('pc.remoteDescription: ', pc.remoteDescription);
console.log('pc.signalingState: ', pc.signalingState);
console.log('pc.iceGatheringState: ', pc.iceGatheringState);
console.log('pc.iceConnectionState: ', pc.iceConnectionState);

pc.createOffer();

console.log('\n\n---------------\n\n');