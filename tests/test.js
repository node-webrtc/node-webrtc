var webrtc = require('../index');

var pc = new webrtc.RTCPeerConnection();
console.log('\n');
console.log('pc:', pc);
console.log('pc Object.keys:' + Object.keys(pc));
pc.createOffer(
  function(offer)
  {
    console.info(offer);
  },
  function(error)
  {
    console.error(error);
  }
);