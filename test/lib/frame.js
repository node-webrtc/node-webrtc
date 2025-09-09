"use strict";

const { i420ToRgba, rgbaToI420 } = require("../..").nonstandard;

class I420Frame {
  constructor(width = 640, height = 480) {
    this.width = width;
    this.height = height;
    this.data = new Uint8Array(this.byteLength);
    Object.freeze(this);
  }

  static fromRgba(rgbaFrame) {
    const i420Frame = new I420Frame(rgbaFrame.width, rgbaFrame.height);
    rgbaToI420(rgbaFrame, i420Frame);
    return i420Frame;
  }

  get byteLength() {
    return (
      this.sizeOfLuminancePlane + // Y
      this.sizeOfChromaPlane + // U
      this.sizeOfChromaPlane
    ); // V
  }

  get sizeOfLuminancePlane() {
    return this.width * this.height;
  }

  get sizeOfChromaPlane() {
    const chromaWidth = Math.floor((this.width + 1) / 2);
    const chromaHeight = Math.floor((this.height + 1) / 2);
    return chromaWidth * chromaHeight;
  }
}

class RgbaFrame {
  constructor(width = 640, height = 480) {
    this.width = width;
    this.height = height;
    this.data = new Uint8ClampedArray(this.byteLength);
    Object.freeze(this);
  }

  static fromI420(i420Frame) {
    const rgbaFrame = new RgbaFrame(i420Frame.width, i420Frame.height);
    i420ToRgba(i420Frame, rgbaFrame);
    return rgbaFrame;
  }

  get byteLength() {
    return this.width * this.height * 4;
  }
}

exports.I420Frame = I420Frame;
exports.RgbaFrame = RgbaFrame;
