/* global Scope */
'use strict';

require('Scope/dist/Scope.js');

const createExample = require('../../lib/browser/example');
const { acquireAudioContext, releaseAudioContext } = require('../../lib/browser/webaudio/refcountedaudiocontext');
const PitchDetector = require('../../lib/common/pitchdetector');

const description = 'This example uses node-webrtc&rsquo;s RTCAudioSource to \
generate a sine wave server-side. Use the number input to change the frequency \
of the server-generated sine wave. Frequency changes are sent to the server \
using RTCDataChannel. Finally, pitch is detected client-side and displayed \
alongside the received waveform.';

const frequencyInput = document.createElement('input');
frequencyInput.type = 'number';
frequencyInput.value = 440;
frequencyInput.min = 0;
frequencyInput.max = 48000 / 2;

const detectedFrequency = document.createElement('p');
detectedFrequency.innerText = 'Detected Frequency:';

async function beforeAnswer(peerConnection) {
  const audioContext = acquireAudioContext();

  const remoteStream = new MediaStream(peerConnection.getReceivers().map(receiver => receiver.track));

  const remoteAudio = document.createElement('audio');
  remoteAudio.srcObject = remoteStream;
  remoteAudio.play();
  document.body.appendChild(remoteAudio);

  const source = audioContext.createMediaStreamSource(remoteStream);

  const canvas = document.createElement('canvas');
  const scope = new Scope(audioContext, canvas);
  source.connect(scope.input);
  scope.start();
  document.body.appendChild(canvas);

  const analyser = audioContext.createAnalyser();
  const bytes = new Uint8Array(analyser.fftSize);
  const pitchDetector = new PitchDetector();
  source.connect(analyser);

  const interval = setInterval(() => {
    const data = {
      samples: bytes,
      bitsPerSample: 8,
      sampleRate: audioContext.sampleRate,
      unsigned: true
    };
    analyser.getByteTimeDomainData(bytes);
    const frequency = pitchDetector.onData(data);
    if (frequency) {
      detectedFrequency.innerText = `Detected Frequency: ${frequency} hz`;
    }
  }, 1000);

  let dataChannel = null;

  function onDataChannel({ channel }) {
    if (channel.label === 'frequency') {
      dataChannel = channel;
    }
  }

  peerConnection.addEventListener('datachannel', onDataChannel);

  function onChange() {
    if (dataChannel.readyState === 'open') {
      dataChannel.send(frequencyInput.value);
    }
  }

  frequencyInput.addEventListener('change', onChange);

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    frequencyInput.removeEventListener('change', onChange);
    peerConnection.removeEventListener('datachannel', onDataChannel);

    remoteAudio.remove();
    remoteAudio.srcObject = null;

    canvas.remove();
    scope.stop();
    source.disconnect(scope.input);

    clearInterval(interval);
    source.disconnect(analyser);
    releaseAudioContext();

    return close.apply(this, arguments);
  };
}

createExample('sine-wave', description, { beforeAnswer });

const frequencyLabel = document.createElement('label');
frequencyLabel.innerText = 'Frequency (hz):';
frequencyLabel.appendChild(frequencyInput);
document.body.appendChild(frequencyLabel);
document.body.appendChild(detectedFrequency);
