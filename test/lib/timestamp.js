'use strict';

function printBitsI420(frame, bits) {
  const { width, height, data } = frame;

  const pixelWidth = width / 8;
  const pixelHeight = height / 8;

  for (let i = 0; i < width * height; i++) {
    const row = Math.floor(i / (8 * pixelWidth * pixelHeight));
    const column = Math.floor(i / pixelWidth) % 8;
    const bit = (row * 8) + column;

    const y = bits[bit] ? 0 : 255;

    data[i] = y;
  }

  for (let i = width * height; i < data.byteLength; i++) {
    data[i] = 128;
  }
}

function readBitsI420(frame) {
  const { width, height, data } = frame;

  const pixelWidth = width / 8;
  const pixelHeight = height / 8;

  const bits = '0'.repeat(64).split('').map(x => Number.parseInt(x, 10));

  for (let i = 0; i < width * height; i++) {
    const row = Math.floor(i / (8 * pixelWidth * pixelHeight));
    const column = Math.floor(i / pixelWidth) % 8;
    const bit = (row * 8) + column;

    const y = data[i];

    bits[bit] += y < 128;
  }

  for (let bit = 0; bit < bits.length; bit++) {
    bits[bit] = bits[bit] >= (pixelWidth * pixelHeight) / 2;
  }

  return bits;
}

exports.printBitsI420 = printBitsI420;
exports.readBitsI420 = readBitsI420;
