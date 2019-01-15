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

function logBits(bits) {
  const [hi, lo] = chunks(bits.map(bit => bit ? '1' : '0'), 32);
  const hiChunks = chunks(hi, 8).map(byte => byte.join('')).join(' ');
  const loChunks = chunks(lo, 8).map(byte => byte.join('')).join(' ');
  console.log('');
  console.log(`  0b ${hiChunks}`);
  console.log(`  0b ${loChunks}`);
  console.log('');
}

function printBits(context, bits) {
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

tape('printBits(context, bits)', t => {
  t.test('it works', t => {
    const width = 8;
    const height = 8;
    const canvas = createCanvas(width, height);
    const context = canvas.getContext('2d');

    printBits(context, toBits(3));

    const { data } = context.getImageData(0, 0, width, height);
    const actualData = [...data];

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

function readBits(context) {
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

tape('readBits(context)', t => {
  t.test('it works', t => {
    const canvas = createCanvas(320, 240);
    const context = canvas.getContext('2d');
    const input = performance.now();
    printBits(context, toBinary64(input));
    const output = fromBinary64(readBits(context));
    t.equal(output, input);
    t.end();
  });
});

function printTimestamp(context) {
  const timestamp = performance.now();
  printBits(context, toBinary64(timestamp));
  return timestamp;
}

function readTimestamp(context) {
  return fromBinary64(readBits(context));
}

tape('readTimestamp(context) === printTimestamp(context)', t => {
  t.test('it works', t => {
    const canvas = createCanvas(640, 480);
    const context = canvas.getContext('2d');
    const expectedTimestamp = printTimestamp(context);
    const actualTimestamp = readTimestamp(context);
    t.equal(actualTimestamp, expectedTimestamp);
    t.end();
  });
});

exports.printBits = printBits;
exports.printTimestamp = printTimestamp;
exports.logBits = logBits;
exports.readBits = readBits;
exports.readTimestamp = readTimestamp;
