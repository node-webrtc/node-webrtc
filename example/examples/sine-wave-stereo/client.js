/* global Scope */
'use strict';

require('Scope/dist/Scope.js');

const createExample = require('../../lib/browser/example');
const { acquireAudioContext, releaseAudioContext } = require('../../lib/browser/webaudio/refcountedaudiocontext');

const description = 'This example uses node-webrtc&rsquo;s RTCAudioSource to \
generate a stereo sine wave server-side. Use the number input to change the \
panning of the server-generated sine wave. Panning changes are sent to the \
server using RTCDataChannel.';

const panningInput = document.createElement('input');
panningInput.type = 'number';
panningInput.value = 50;
panningInput.min = 0;
panningInput.max = 100;

const canvases = document.createElement('div');
canvases.className = 'grid';

async function beforeAnswer(peerConnection) {
  const audioContext = acquireAudioContext();

  const remoteStream = new MediaStream(peerConnection.getReceivers().map(receiver => receiver.track));

  const remoteAudio = document.createElement('audio');
  remoteAudio.srcObject = remoteStream;
  remoteAudio.play();
  document.body.appendChild(remoteAudio);

  const source = audioContext.createMediaStreamSource(remoteStream);
  const splitter = audioContext.createChannelSplitter(2);
  source.connect(splitter);

  const leftCanvas = document.createElement('canvas');
  const rightCanvas = document.createElement('canvas');
  const leftScope = new Scope(audioContext, leftCanvas);
  const rightScope = new Scope(audioContext, rightCanvas);
  splitter.connect(leftScope.input, 0, 0);
  splitter.connect(rightScope.input, 1, 0);
  leftScope.start();
  rightScope.start();
  canvases.appendChild(leftCanvas);
  canvases.appendChild(rightCanvas);

  let dataChannel = null;

  function onDataChannel({ channel }) {
    if (channel.label === 'panning') {
      dataChannel = channel;
    }
  }

  peerConnection.addEventListener('datachannel', onDataChannel);

  function onChange() {
    if (dataChannel.readyState === 'open') {
      dataChannel.send(panningInput.value);
    }
  }

  panningInput.addEventListener('change', onChange);

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    panningInput.removeEventListener('change', onChange);
    peerConnection.removeEventListener('datachannel', onDataChannel);

    remoteAudio.remove();
    remoteAudio.srcObject = null;

    leftCanvas.remove();
    rightCanvas.remove();
    leftScope.stop();
    rightScope.stop();
    splitter.disconnect(leftScope.input, 0);
    splitter.disconnect(rightScope.input, 1);
    source.disconnect(splitter);

    releaseAudioContext();

    return close.apply(this, arguments);
  };
}

createExample('sine-wave-stereo', description, { beforeAnswer, stereo: true });

const panningLabel = document.createElement('label');
panningLabel.innerText = 'Panning (0–100):';
panningLabel.appendChild(panningInput);
document.body.appendChild(panningLabel);

document.body.appendChild(canvases);
