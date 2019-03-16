'use strict';

function beforeOffer(peerConnection) {
  const dataChannel = peerConnection.createDataChannel('ping-pong');

  function onMessage({ data }) {
    if (data === 'ping') {
      dataChannel.send('pong');
    }
  }

  dataChannel.addEventListener('message', onMessage);

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    dataChannel.removeEventListener('message', onMessage);
    return close.apply(this, arguments);
  };
}

module.exports = { beforeOffer };
