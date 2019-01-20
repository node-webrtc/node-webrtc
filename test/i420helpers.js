'use strict';

const tape = require('tape');

const { i420ToRgba } = require('..').nonstandard;
const { I420Frame } = require('./lib/frame');

tape('i420ToRgba(i420Frame, rgbaBuffer)', t => {
  t.test('it works', t => {
    const width = 640;
    const height = 480;
    const i420Frame = new I420Frame(width, height);
    const rgbaData = new Uint8ClampedArray(width * height * 4);
    const rgbaFrame = { width, height, data: rgbaData };

    blackenI420Frame(i420Frame);
    i420ToRgba(i420Frame, rgbaFrame);
    // TODO(mroberts): Test the result.

    whitenI420Frame(i420Frame);
    i420ToRgba(i420Frame, rgbaFrame);
    // TODO(mroberts): Test the result.

    t.end();
  });
});

function blackenI420Frame(i420Frame) {
  const startOfUPlane = i420Frame.sizeOfLuminancePlane;
  for (let i = 0; i < i420Frame.byteLength; i++) {
    i420Frame.data[i] = i < startOfUPlane ? 0 : 128;
  }
}

function whitenI420Frame(i420Frame) {
  const startOfUPlane = i420Frame.sizeOfLuminancePlane;
  for (let i = 0; i < i420Frame.byteLength; i++) {
    i420Frame.data[i] = i < startOfUPlane ? 255 : 128;
  }
}
