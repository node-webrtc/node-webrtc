'use strict';

const { rgb } = require('color-space');

class I420Frame {
  constructor(width = 640, height = 480) {
    this.width = width;
    this.height = height;
    this.data = new Uint8ClampedArray(this.byteLength);
    Object.freeze(this);
  }

  static fromArgb32(argb32, width, height) {
    const frame = new I420Frame(width, height);
    argb32ToI420(argb32, frame.data, width, height);
    return frame;
  }

  get byteLength() {
    return this.sizeOfLuminancePlane  // Y
         + this.sizeOfChromaPlane     // U
         + this.sizeOfChromaPlane;    // V
  }

  get sizeOfLuminancePlane() {
    return this.width * this.height;
  }

  get sizeOfChromaPlane() {
    return this.sizeOfLuminancePlane / 4;
  }
}

// NOTE(mroberts): I'm not sure this is correct; we should just bind libyuv.
function argb32ToI420(argb32, i420, width, height) {
  if (width % 2) {
    throw new Error(`Width of ${width} is not a multiple of 2`);
  } else if (height % 2) {
    throw new Error(`Height of ${height} is not a multiple of 2`);
  }

  const halfWidth = width / 2;
  const halfHeight = height / 2;

  const expectedArgb32Size = width * height * 4;
  if (argb32.byteLength !== expectedArgb32Size) {
    throw new Error(`Expected a .byteLength of ${expectedArgb32Size} for the \
ARGB32 Uint8Array, not ${argb32.byteLength}`);
  }

  const expectedI420Size = 1.5 * width * height;
  if (i420.byteLength !== expectedI420Size) {
    throw new Error(`Expected a .byteLength of ${expectedI420Size} for the \
I420 Uint8Array, not ${i420.byteLength}`);
  }

  const sizeOfLuminancePlane = width * height;
  const sizeOfChromaPlane = sizeOfLuminancePlane / 4;
  const startOfUPlane = sizeOfLuminancePlane;
  const startOfVPlane = startOfUPlane + sizeOfChromaPlane;

  const redChannel = 0;
  const greenChannel = 1;
  const blueChannel = 2;

  for (let halfRow = 0; halfRow < halfHeight; halfRow++) {
    const fullRow = halfRow * 2;

    for (let halfColumn = 0; halfColumn < halfWidth; halfColumn++) {
      const fullColumn = halfColumn * 2;

      const halfPixel = halfRow * halfWidth + halfColumn;

      const fullPixel1 = (fullRow + 0) * width + fullColumn + 0;
      const fullPixel2 = (fullRow + 0) * width + fullColumn + 1;
      const fullPixel3 = (fullRow + 1) * width + fullColumn + 0;
      const fullPixel4 = (fullRow + 1) * width + fullColumn + 1;

      const rgb1 = [
        argb32[fullPixel1 * 4 + redChannel],
        argb32[fullPixel1 * 4 + greenChannel],
        argb32[fullPixel1 * 4 + blueChannel]
      ];

      const rgb2 = [
        argb32[fullPixel2 * 4 + redChannel],
        argb32[fullPixel2 * 4 + greenChannel],
        argb32[fullPixel2 * 4 + blueChannel]
      ];

      const rgb3 = [
        argb32[fullPixel3 * 4 + redChannel],
        argb32[fullPixel3 * 4 + greenChannel],
        argb32[fullPixel3 * 4 + blueChannel]
      ];

      const rgb4 = [
        argb32[fullPixel4 * 4 + redChannel],
        argb32[fullPixel4 * 4 + greenChannel],
        argb32[fullPixel4 * 4 + blueChannel]
      ];

      const [y1, u1, v1] = rgb.yuv(rgb1);
      const [y2, u2, v2] = rgb.yuv(rgb2);
      const [y3, u3, v3] = rgb.yuv(rgb3);
      const [y4, u4, v4] = rgb.yuv(rgb4);

      i420[fullPixel1] = Math.round(y1 * 255);
      i420[fullPixel2] = Math.round(y2 * 255);
      i420[fullPixel3] = Math.round(y3 * 255);
      i420[fullPixel4] = Math.round(y4 * 255);

      const u = (u1 + u2 + u3 + u4) / 4;
      const v = (v1 + v2 + v3 + v4) / 4;

      i420[startOfUPlane + halfPixel] = Math.round((u + 0.5) * 255);
      i420[startOfVPlane + halfPixel] = Math.round((v + 0.5) * 255);
    }
  }
}

exports.argb32ToI420 = argb32ToI420;
exports.I420Frame = I420Frame;
