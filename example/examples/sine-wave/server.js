'use strict';

const RTCAudioSourceSineWave = require('../../lib/server/webrtc/rtcaudiosourcesinewave');

function beforeOffer(peerConnection) {
  const source = new RTCAudioSourceSineWave();
  const track = source.createTrack();
  peerConnection.addTrack(track);

  const dataChannel = peerConnection.createDataChannel('frequency');

  function onMessage({ data }) {
    const frequency = Number.parseFloat(data);
    source.setFrequency(frequency);
  }

  dataChannel.addEventListener('message', onMessage);

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    dataChannel.removeEventListener('message', onMessage);
    track.stop();
    source.close();
    return close.apply(this, arguments);
  };
}

module.exports = { beforeOffer };
