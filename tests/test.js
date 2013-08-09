var webrtc = require('../index');

console.log(webrtc);

var pc = new webrtc.RTCPeerConnection();

console.log('pc.localDescription: ' + pc.localDescription);
console.log('pc.remoteDescription: ' + pc.remoteDescription);
console.log('pc.signalingState: ' + pc.signalingState);
console.log('pc.iceState: ' + pc.iceState);
