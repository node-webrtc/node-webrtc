'use strict';

class PitchDetector {
  constructor(options = {}) {
    options = {
      samples: 1024,
      ...options
    };

    const { samples } = options;
    if (samples.length < 2) {
      throw new Error('samples must be greater than 2');
    }

    const crossings = [];

    let changed = false;
    let lastTime = 0;
    let frequency = null;
    let isPositive = true;
    let sampleRate = null;

    this.onData = data => {
      sampleRate = data.sampleRate;
      const timePerSample = 1 / sampleRate;
      const zero = data.unsigned ? (2 ** data.bitsPerSample) / 2 : 0;
      data.samples.forEach((sample, i) => {
        if (isPositive && sample < zero) {
          changed = true;
          isPositive = false;
          const crossing = lastTime + i * timePerSample;
          enqueue(crossings, crossing, samples);
        } else if (!isPositive && sample > zero) {
          isPositive = true;
        }
      });
      lastTime += data.samples.length * timePerSample;
      if (changed && crossings.length >= 2) {
        changed = false;
        const averagePeriod = averageDifference(crossings);
        const averageFrequency = 1 / averagePeriod;
        frequency = averageFrequency;
        return frequency === null ? null : Math.floor(frequency);
      }
      return null;
    };

    Object.defineProperties(this, {
      frequency: {
        get() {
          return frequency;
        }
      },
      sampleRate: {
        get() {
          return sampleRate;
        }
      }
    });
  }
}

function averageDifference(xs) {
  return pairs(xs).reduce((y, [x1, x2]) => (x2 - x1) + y, 0) / xs.length;
}

function enqueue(xs, x, n) {
  if (xs.length === n) {
    xs.shift();
  }
  xs.push(x);
}

function pairs(xs) {
  return xs.slice(1).map((x, i) => [xs[i], x]);
}

module.exports = PitchDetector;
