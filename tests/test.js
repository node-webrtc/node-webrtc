var webrtc = require('../index');

var pc = new webrtc.RTCPeerConnection();

console.log('pc Object.keys: ' + Object.keys(pc));

console.log('pc.localDescription: ', pc.localDescription);
console.log('pc.remoteDescription: ', pc.remoteDescription);
console.log('pc.signalingState: ', pc.signalingState);
console.log('pc.iceGatheringState: ', pc.iceGatheringState);
console.log('pc.iceConnectionState: ', pc.iceConnectionState);

pc.createOffer(
  function(sdp){
    console.log('sdp', sdp);
    pc.setLocalDescription(
      sdp,
      function() {
        console.log('pc.localDescription: ', pc.localDescription);
        console.log('pc.signalingState: ', pc.signalingState);
      },
      function(err) {
        console.log('error', err);
      }
    );
  },
  function(err){
    console.log('error', err);
  },
  {}
);
