'use strict';

const { RTCAudioSource } = require('../../../..').nonstandard;

const twoPi = 2 * Math.PI;

class RTCAudioSourceSineWave {
  constructor(options = {}) {
    options = {
      frequency: 440,
      sampleRate: 48000,
      schedule: setTimeout,
      unschedule: clearTimeout,
      ...options
    };

    const bitsPerSample = 16;
    const numberOfChannels = 1;
    const { sampleRate } = options;
    const source = new RTCAudioSource();

    const numberOfFrames = sampleRate / 100;

    const audioData = new Int16Array(numberOfFrames * numberOfChannels);

    const data = {
      audioData,
      sampleRate,
      bitsPerSample,
      numberOfChannels,
      numberOfFrames
    };

    let frequency = options.frequency;
    let secondsPerSample = 1 / sampleRate;
    let time = 0;

    function next() {
      for (let i = 0; i < numberOfFrames; i++, time += secondsPerSample) {
        audioData[i] = Math.sin(twoPi * frequency * time) * 32767;
      }
      source.onData(data);
      // eslint-disable-next-line
      scheduled = options.schedule(next);
    }

    let scheduled = options.schedule(next);

    this.close = () => {
      options.unschedule(scheduled);
      scheduled = null;
    };

    this.createTrack = () => {
      return source.createTrack();
    };

    this.setFrequency = newFrequency => {
      frequency = newFrequency;
    };

    Object.defineProperties(this, {
      frequency: {
        get() {
          return frequency;
        }
      }
    });
  }
}

module.exports = RTCAudioSourceSineWave;
