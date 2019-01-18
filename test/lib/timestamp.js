'use strict';

const { createCanvas } = require('canvas');
const { rgb } = require('color-space');
const { performance } = require('perf_hooks');
const tape = require('tape');

const {
  chunks,
  flatMap,
  fromBinary64,
  toBinary64,
  toBits
} = require('./bits');

const { I420Frame } = require('./frame');

function logBits(bits) {
  const [hi, lo] = chunks(bits.map(bit => bit ? '1' : '0'), 32);
  const hiChunks = chunks(hi, 8).map(byte => byte.join('')).join(' ');
  const loChunks = chunks(lo, 8).map(byte => byte.join('')).join(' ');
  console.log('');
  console.log(`  0b ${hiChunks}`);
  console.log(`  0b ${loChunks}`);
  console.log('');
}

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

tape('printBitsI420(frame, bits)', t => {
  t.test('it works', t => {
    const width = 8;
    const height = 8;
    const frame = new I420Frame(width, height);
    const bits = [
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 1, 1,
    ];
    printBitsI420(frame, bits);
    t.deepEqual(
      frame.data.slice(0, frame.sizeOfLuminancePlane),
      bits.map(bit => bit ? 0 : 255)
    );
    t.ok(
      frame.data
        .slice(frame.sizeOfLuminancePlane)
        .every(x => x === 128)
    );
    t.end();
  });
});

function printBitsCanvas(context, bits) {
  const { canvas: { width, height } } = context;
  context.fillStyle = 'white';
  context.fillRect(0, 0, width, height);

  const bitWidth = width / 8;
  const bitHeight = height / 8;
  context.fillStyle = 'black';
  bits.forEach((bit, i) => {
    const row = Math.floor(i / 8);
    const column = i % 8;
    const x = column * bitWidth;
    const y = row * bitHeight;
    if (bit) {
      context.fillRect(x, y, bitWidth, bitHeight);
    }
  });
}

tape('printBitsCanvas(context, bits)', t => {
  t.test('it works', t => {
    const width = 8;
    const height = 8;
    const canvas = createCanvas(width, height);
    const context = canvas.getContext('2d');

    printBitsCanvas(context, toBits(3));

    const { data } = context.getImageData(0, 0, width, height);
    const actualData = data.slice(0);

    // eslint-disable-next-line
    const I = [  0,   0,   0, 255];
    const O = [255, 255, 255, 255];
    const expectedData = flatMap([
      O, O, O, O, O, O, O, O,
      O, O, O, O, O, O, O, O,
      O, O, O, O, O, O, O, O,
      O, O, O, O, O, O, O, O,
      O, O, O, O, O, O, O, O,
      O, O, O, O, O, O, O, O,
      O, O, O, O, O, O, O, O,
      O, O, O, O, O, O, I, I
    ], rgba => rgba);

    t.deepEqual(actualData, expectedData);

    t.end();
  });
});

function readBitsI420(frame) {
  const { width, height, data } = frame;

  const pixelWidth = width / 8;
  const pixelHeight = height / 8;

  const bits = toBits(0);

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

function readBitsCanvas(context) {
  const { canvas: { width, height } } = context;
  const { data } = context.getImageData(0, 0, width, height);

  const bits = [
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0
  ];

  const pixelWidth = width / 8;
  const pixelHeight = height / 8;

  for (let i = 0; i < width * height; i++) {
    const row = Math.floor(i / (8 * pixelWidth * pixelHeight));
    const column = Math.floor(i / pixelWidth) % 8;
    const bit = (row * 8) + column;

    const r = data[i * 4];
    const g = data[i * 4 + 1];
    const b = data[i * 4 + 2];
    const l = rgb.hsl([r, g, b])[2];

    bits[bit] += l < 50;
  }

  for (let bit = 0; bit < 64; bit++) {
    bits[bit] = bits[bit] >= (pixelWidth * pixelHeight) / 2;
  }

  return bits;
}

tape('readBitsCanvas(context)', t => {
  t.test('it works', t => {
    const canvas = createCanvas(320, 240);
    const context = canvas.getContext('2d');
    const input = performance.now();
    printBitsCanvas(context, toBinary64(input));
    const output = fromBinary64(readBitsCanvas(context));
    t.equal(output, input);
    t.end();
  });
});

function printTimestampCanvas(context) {
  const timestamp = performance.now();
  printBitsCanvas(context, toBinary64(timestamp));
  return timestamp;
}

function printTimestampI420(frame) {
  const timestamp = performance.now();
  printBitsI420(frame, toBinary64(timestamp));
  return timestamp;
}

function readTimestampCanvas(context) {
  return fromBinary64(readBitsCanvas(context));
}

function readTimestampI420(frame) {
  return fromBinary64(readBitsI420(frame));
}

tape('readTimestampCanvas(context) === printTimestampCanvas(context)', t => {
  t.test('it works', t => {
    const canvas = createCanvas(640, 480);
    const context = canvas.getContext('2d');
    const expectedTimestamp = printTimestampCanvas(context);
    const actualTimestamp = readTimestampCanvas(context);
    t.equal(actualTimestamp, expectedTimestamp);
    t.end();
  });
});

tape('readTimestampI420(frame) === printTimestampI420(frame)', t => {
  t.test('it works', t => {
    const frame = new I420Frame();
    const expectedTimestamp = printTimestampI420(frame);
    const actualTimestamp = readTimestampI420(frame);
    t.equal(actualTimestamp, expectedTimestamp);
    t.end();
  });
});

exports.printBitsCanvas = printBitsCanvas;
exports.printBitsI420 = printBitsI420;
exports.printTimestampCanvas = printTimestampCanvas;
exports.printTimestampI420 = printTimestampI420;
exports.logBits = logBits;
exports.readBitsCanvas = readBitsCanvas;
exports.readBitsI420 = readBitsI420;
exports.readTimestampCanvas = readTimestampCanvas;
exports.readTimestampI420 = readTimestampI420;
