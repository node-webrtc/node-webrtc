'use strict';

const RTCAudioSourceSineWave = require('../../lib/server/webrtc/rtcaudiosourcesinewave');

function beforeOffer(peerConnection) {
  const source = new RTCAudioSourceSineWave({
    channelCount: 2,
    panning: 50
  });
  const track = source.createTrack();
  peerConnection.addTrack(track);

  const dataChannel = peerConnection.createDataChannel('panning');

  function onMessage({ data }) {
    const panning = Number.parseFloat(data);
    source.setPanning(panning);
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
