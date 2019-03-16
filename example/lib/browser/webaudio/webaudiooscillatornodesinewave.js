'use strict';

const { acquireAudioContext, releaseAudioContext } = require('./refcountedaudiocontext');

class WebAudioOscillatorNodeSineWave {
  constructor(options = {}) {
    options = {
      audioContext: null,
      frequency: 440,
      ...options
    };

    const isUsingDefaultAudioContext = !options.audioContext;
    const audioContext = options.audioContext || acquireAudioContext();

    const oscillatorNode = audioContext.createOscillator();
    oscillatorNode.frequency.setValueAtTime(options.frequency, audioContext.currentTime);
    oscillatorNode.connect(audioContext.destination);
    oscillatorNode.start();

    this.node = oscillatorNode;

    this.close = () => {
      oscillatorNode.stop();
      if (isUsingDefaultAudioContext) {
        releaseAudioContext();
      }
    };

    this.setFrequency = frequency => {
      oscillatorNode.frequency.setValueAtTime(frequency, audioContext.currentTime);
    };
  }
}

module.exports = WebAudioOscillatorNodeSineWave;
