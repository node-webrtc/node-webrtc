'use strict';

const { rgbaToI420 } = require('../..').nonstandard;

class I420Frame {
  constructor(width = 640, height = 480) {
    this.width = width;
    this.height = height;
    this.data = new Uint8ClampedArray(this.byteLength);
    Object.freeze(this);
  }

  static fromRgba(rgbaFrame) {
    const i420Frame = new I420Frame(rgbaFrame.width, rgbaFrame.height);
    rgbaToI420(rgbaFrame, i420Frame);
    return i420Frame;
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

exports.I420Frame = I420Frame;
