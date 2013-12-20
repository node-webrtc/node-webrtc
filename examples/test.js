var webrtc = require('../index');

var pc = new webrtc.RTCPeerConnection();
console.log('\n');

var channel = pc.createDataChannel("test", {
  ordered: false
});

console.log("channel:", channel);

pc.close();
pc = null;