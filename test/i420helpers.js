'use strict';

const tape = require('tape');

const { i420ToArgb32 } = require('..').nonstandard;
const { I420Frame } = require('./lib/frame');

tape('i420ToArgb32(i420Frame, argb32Buffer)', t => {
  t.test('it works', t => {
    const i420Frame = new I420Frame();
    const argb32Buffer = new Uint8ClampedArray(i420Frame.width * i420Frame.height * 4);

    blackenI420Frame(i420Frame);
    i420ToArgb32(i420Frame, argb32Buffer);
    // TODO(mroberts): Test the result.

    whitenI420Frame(i420Frame);
    i420ToArgb32(i420Frame, argb32Buffer);
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
