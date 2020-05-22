'use strict';

const tape = require('tape');

const { i420ToRgba, rgbaToI420 } = require('..').nonstandard;

const { I420Frame, RgbaFrame } = require('./lib/frame');

tape('i420ToRgba(i420Frame, rgbaFrame)', t => {
  t.test('it works', t => {
    const width = 160;
    const height = 120;

    const i420Frame = new I420Frame(width, height);
    const rgbaFrame = new RgbaFrame(width, height);

    blackenI420Frame(i420Frame);
    i420ToRgba(i420Frame, rgbaFrame);
    t.ok(everyRgba(rgbaFrame, [0, 0, 0, 255]),
      'converting a black I420 frame to RGBA works');

    whitenI420Frame(i420Frame);
    i420ToRgba(i420Frame, rgbaFrame);
    t.ok(everyRgba(rgbaFrame, [255, 255, 255, 255]),
      'converting a white I420 frame to RGBA works');

    setYuv(i420Frame, [173, 143, 31]);
    i420ToRgba(i420Frame, rgbaFrame);
    t.ok(everyRgba(rgbaFrame, [28, 255, 213, 255]),
      'converting a turquoise I420 frame to RGBA works');

    t.end();
  });
});

tape('rgbaToI420(rgbaFrame, i420Frame)', t => {
  t.test('it works', t => {
    const width = 160;
    const height = 120;

    const rgbaFrame = new RgbaFrame(width, height);
    const i420Frame = new I420Frame(width, height);

    blackenRgbaFrame(rgbaFrame);
    rgbaToI420(rgbaFrame, i420Frame);
    t.ok(everyYuv(i420Frame, [16, 128, 128]),
      'converting a black RGBA frame to I420 works');

    whitenRgbaFrame(rgbaFrame);
    rgbaToI420(rgbaFrame, i420Frame);
    t.ok(everyYuv(i420Frame, [235, 128, 128]),
      'converting a white RGBA frame to I420 works');

    setRgba(rgbaFrame, [28, 255, 213, 255]);
    rgbaToI420(rgbaFrame, i420Frame);
    t.ok(everyYuv(i420Frame, [173, 143, 31]),
      'converting a turquoise RGBA frame to I420 works');

    t.end();
  });
});

function setYuv(i420Frame, yuv) {
  for (let i = 0; i < i420Frame.byteLength; i++) {
    if (i < i420Frame.sizeOfLuminancePlane) {
      i420Frame.data[i] = yuv[0];
    } else if (i < i420Frame.sizeOfLuminancePlane + i420Frame.sizeOfChromaPlane) {
      i420Frame.data[i] = yuv[1];
    } else {
      i420Frame.data[i] = yuv[2];
    }
  }
}

function blackenI420Frame(i420Frame) {
  setYuv(i420Frame, [0, 128, 128]);
}

function whitenI420Frame(i420Frame) {
  setYuv(i420Frame, [255, 128, 128]);
}

function setRgba(rgbaFrame, rgba) {
  for (let i = 0; i < rgbaFrame.width * rgbaFrame.height; i++) {
    rgbaFrame.data[i * 4 + 0] = rgba[0];
    rgbaFrame.data[i * 4 + 1] = rgba[1];
    rgbaFrame.data[i * 4 + 2] = rgba[2];
    rgbaFrame.data[i * 4 + 3] = rgba[3];
  }
}

function blackenRgbaFrame(rgbaFrame) {
  setRgba(rgbaFrame, [0, 0, 0, 255]);
}

function whitenRgbaFrame(rgbaFrame) {
  setRgba(rgbaFrame, [255, 255, 255, 255]);
}

function everyRgba(rgbaFrame, rgba) {
  for (let i = 0; i < rgbaFrame.width * rgbaFrame.height; i++) {
    const r = rgbaFrame.data[i * 4 + 0];
    const g = rgbaFrame.data[i * 4 + 1];
    const b = rgbaFrame.data[i * 4 + 2];
    const a = rgbaFrame.data[i * 4 + 3];
    if (r !== rgba[0]
      || g !== rgba[1]
      || b !== rgba[2]
      || a !== rgba[3]) {
      console.log([r, g, b, a], rgba);
      return false;
    }
  }
  return true;
}

function everyYuv(i420Frame, yuv) {
  for (let i = 0; i < i420Frame.byteLength; i++) {
    if (i < i420Frame.sizeOfLuminancePlane) {
      const y = i420Frame.data[i];
      if (y !== yuv[0]) {
        console.log('y', y, yuv[0]);
        return false;
      }
    } else if (i < i420Frame.sizeOfLuminancePlane + i420Frame.sizeOfChromaPlane) {
      const u = i420Frame.data[i];
      if (u !== yuv[1]) {
        console.log('u', u, yuv[1]);
        return false;
      }
    } else {
      const v = i420Frame.data[i];
      if (v !== yuv[2]) {
        console.log('v', v, yuv[2]);
        return false;
      }
    }
  }
  return true;
}
