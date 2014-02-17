var RTCPeerConnection = require('..').RTCPeerConnection;


var pc = new RTCPeerConnection({
    iceServers: [{url:'stun:stun.l.google.com:19302'}]
  },
  {
    'optional': [{DtlsSrtpKeyAgreement: false},
                 {RtpDataChannels: true}]
  });

var channel = pc.createDataChannel("test", {
  ordered: false
});
channel.onopen = function() {
  console.log('channel open');
}
channel.onclose = function() {
  console.log('channel close');
}

setTimeout(function() {
  // channel.close();
  // channel = null;
  pc.close();
  // pc = null;
}, 1);