var webrtc = require('../index');

var pc = new webrtc.RTCPeerConnection();

console.log('\npc Object.keys: ' + Object.keys(pc), '\n');

pc.createOffer(
  function(sdp){
    console.log('\ncreated offer sdp:', sdp, '\n');
    pc.setLocalDescription(
      sdp,
      function() {
        console.log('\nlocal description set:', pc.localDescription, '\n');
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
pc.onsignalingstatechange = function(state) {
  console.log('\nsignaling state changed:', state, '\n');
};
