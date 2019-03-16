'use strict';

const createExample = require('../../lib/browser/example');

const description = 'This example sends a &ldquo;ping&rdquo; from the client \
over an RTCDataChannel. Upon receipt, node-webrtc responds with a \
&ldquo;pong&rdquo;. Open the Console to see the pings and pongs&hellip;';

function beforeAnswer(peerConnection) {
  let dataChannel = null;
  let interval = null;

  function onMessage({ data }) {
    if (data === 'pong') {
      console.log('received pong');
    }
  }

  function onDataChannel({ channel }) {
    if (channel.label !== 'ping-pong') {
      return;
    }

    dataChannel = channel;
    dataChannel.addEventListener('message', onMessage);

    interval = setInterval(() => {
      console.log('sending ping');
      dataChannel.send('ping');
    }, 1000);
  }

  peerConnection.addEventListener('datachannel', onDataChannel);

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    if (dataChannel) {
      dataChannel.removeEventListener('message', onMessage);
    }
    if (interval) {
      clearInterval(interval);
    }
    return close.apply(this, arguments);
  };
}

createExample('ping-pong', description, { beforeAnswer });
