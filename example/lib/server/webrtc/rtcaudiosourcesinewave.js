'use strict';

const { RTCAudioSource } = require('../../../..').nonstandard;

const twoPi = 2 * Math.PI;

class RTCAudioSourceSineWave {
  constructor(options = {}) {
    options = {
      frequency: 440,
      numberOfChannels: 1,
      panning: null,
      sampleRate: 48000,
      schedule: setTimeout,
      unschedule: clearTimeout,
      ...options
    };

    const {
      numberOfChannels,
      sampleRate
    } = options;

    if (numberOfChannels !== 1 && numberOfChannels !== 2) {
      throw new Error('numberOfChannels must be 1 or 2');
    }

    const bitsPerSample = 16;
    const maxValue = Math.pow(2, bitsPerSample) / 2 - 1;
    const numberOfFrames = sampleRate / 100;
    const secondsPerSample = 1 / sampleRate;
    const source = new RTCAudioSource();
    const audioData = new Int16Array(numberOfFrames * numberOfChannels);

    const data = {
      audioData,
      sampleRate,
      bitsPerSample,
      numberOfChannels,
      numberOfFrames
    };

    const a = [1, 1];

    let {
      frequency,
      panning
    } = options;

    let time = 0;

    function next() {
      for (let i = 0; i < numberOfFrames; i++, time += secondsPerSample) {
        for (let j = 0; j < numberOfChannels; j++) {
          audioData[i * numberOfChannels + j] = a[j] * Math.sin(twoPi * frequency * time) * maxValue;
        }
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

    this.setPanning = newPanning => {
      if (numberOfChannels === 1) {
        return;
      }
      panning = newPanning;
      a[0] = 1 - (panning / 100);
      a[1] = 1 - ((100 - panning) / 100);
    };

    this.setPanning(panning);

    Object.defineProperties(this, {
      frequency: {
        get() {
          return frequency;
        }
      },
      panning: {
        get() {
          return panning;
        }
      }
    });
  }
}

module.exports = RTCAudioSourceSineWave;
