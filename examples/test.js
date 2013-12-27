var RTCPeerConnection;
var window = window || undefined;

if(window && window.mozRTCPeerConnection)
  RTCPeerConnection = window.mozRTCPeerConnection;
else if(window && window.webkitRTCPeerConnection)
  RTCPeerConnection = window.webkitRTCPeerConnection;
else if(window && window.RTCPeerConnection)
  RTCPeerConnection = window.RTCPeerConnection
else
  RTCPeerConnection = require('../index').RTCPeerConnection;

var pc = new RTCPeerConnection({
    iceServers: [{url:'stun:stun.l.google.com:19302'}]
  },
  {
    'optional': [{DtlsSrtpKeyAgreement: false},
                 {RtpDataChannels: true}]
  });
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

channel.close();
pc.close();
pc = null;
