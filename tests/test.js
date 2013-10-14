var webrtc = require('../index');

var pc = new webrtc.RTCPeerConnection();
console.log('\n');
console.info('pc:', pc);
console.info('pc.localDescription:', pc.localDescription);
console.info('pc.remoteDescription:', pc.remoteDescription);
console.info('pc.signalingState:', pc.signalingState);
console.info('pc.iceConnectionState:', pc.iceConnectionState);
console.info('pc.iceGatheringState:', pc.iceGatheringState);