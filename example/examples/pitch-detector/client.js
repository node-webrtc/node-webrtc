/* global Scope */
'use strict';

require('Scope/dist/Scope.js');

const createExample = require('../../lib/browser/example');
const { acquireAudioContext, releaseAudioContext } = require('../../lib/browser/webaudio/refcountedaudiocontext');
const WebAudioOscillatorNodeSineWave = require('../../lib/browser/webaudio/webaudiooscillatornodesinewave');

const description = 'This example uses node-webrtc&rsquo;s RTCAudioSink to \
implement simple pitch detection server-side. The client generates a sine \
wave, and the server communicates the pitch it detects using RTCDataChannel. \
Use the number input to change the frequency of the client-generated sine \
wave.';

const frequencyInput = document.createElement('input');
frequencyInput.type = 'number';
frequencyInput.value = 440;
frequencyInput.min = 0;
frequencyInput.max = 48000 / 2;

const detectedFrequency = document.createElement('p');
detectedFrequency.innerText = 'Detected Frequency:';

function beforeAnswer(peerConnection) {
  const audioContext = acquireAudioContext();
  const webAudioSineWave = new WebAudioOscillatorNodeSineWave({
    frequency: frequencyInput.value
  });

  function onChange() {
    console.log(frequencyInput.value);
    webAudioSineWave.setFrequency(frequencyInput.value);
  }

  frequencyInput.addEventListener('change', onChange);

  const streamNode = audioContext.createMediaStreamDestination();
  webAudioSineWave.node.connect(streamNode);
  const { stream } = streamNode;
  stream.getTracks().forEach(track => peerConnection.addTrack(track, stream));

  const canvas = document.createElement('canvas');
  const scope = new Scope(audioContext, canvas);
  webAudioSineWave.node.connect(scope.input);
  scope.start();
  document.body.appendChild(canvas);

  let dataChannel = null;

  function onMessage({ data }) {
    detectedFrequency.innerText = `Detected Frequency: ${data} hz`;
  }

  function onDataChannel({ channel }) {
    if (channel.label !== 'frequency') {
      return;
    }

    dataChannel = channel;
    dataChannel.addEventListener('message', onMessage);
  }

  peerConnection.addEventListener('datachannel', onDataChannel);

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    frequencyInput.removeEventListener('change', onChange);

    canvas.remove();
    scope.stop();
    webAudioSineWave.node.disconnect(scope.input);

    stream.getTracks().forEach(track => track.stop());
    webAudioSineWave.node.disconnect(streamNode);
    webAudioSineWave.close();
    releaseAudioContext();

    if (dataChannel) {
      dataChannel.removeEventListener('message', onMessage);
    }

    return close.apply(this, arguments);
  };
}

createExample('pitch-detector', description, { beforeAnswer });

const frequencyLabel = document.createElement('label');
frequencyLabel.innerText = 'Frequency (hz):';
frequencyLabel.appendChild(frequencyInput);
document.body.appendChild(frequencyLabel);
document.body.appendChild(detectedFrequency);
