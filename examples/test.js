var webrtc = require('../index');

var pc = new webrtc.RTCPeerConnection();
console.log('\n');

var channel = pc.createDataChannel("test", {
  ordered: false
});
console.log("channel:", channel);
channel.onopen = function() {
  console.log('channel open');
}
channel.onclose = function() {
  console.log('channel close');
}

pc.close();
//pc = null;