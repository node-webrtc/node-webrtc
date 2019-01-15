'use strict';

// const tape = require('tape');
function tape() {}
const { performance } = require('perf_hooks');

/**
 * Convert a number to a binary64 bitstring.
 * @param {number} n - the number
 * @returns {Array<bool>} binary64 - the binary64 bitstring
 */
function toBinary64(n) {
  const float64Array = new Float64Array(1);
  float64Array[0] = n;

  const uint8Array = new Uint8Array(float64Array.buffer);

  return flatMap(uint8Array, byteToBits);
}

tape('toBinary64(n)', t => {
  t.test('it works for n=0', t => {
    const binary64 = toBinary64(0);
    t.equal(binary64.length, 64, 'returns a 64-bit bitstring');
    t.ok(binary64.every(bit => bit === false), 'every bit is false');
    t.end();
  });
});

/**
 * Convert a binary64 bitstring to a number.
 * @param {Array<bool>} binary64 - the binary64 bitstring
 * @returns {number} n - the number
 */
function fromBinary64(binary64) {
  if (binary64.length !== 64) {
    throw new Error('Expected 64 bits');
  }

  const uint8Array = new Uint8Array(8);
  uint8Array[0] = bitsToByte(binary64.slice( 0,  0 + 8));
  uint8Array[1] = bitsToByte(binary64.slice( 8,  8 + 8));
  uint8Array[2] = bitsToByte(binary64.slice(16, 16 + 8));
  uint8Array[3] = bitsToByte(binary64.slice(24, 24 + 8));
  uint8Array[4] = bitsToByte(binary64.slice(32, 32 + 8));
  uint8Array[5] = bitsToByte(binary64.slice(40, 40 + 8));
  uint8Array[6] = bitsToByte(binary64.slice(48, 48 + 8));
  uint8Array[7] = bitsToByte(binary64.slice(56, 56 + 8));

  const float64Array = new Float64Array(uint8Array.buffer);
  return float64Array[0];
}

tape('fromBinary64(toBinary64(n))', t => {
  t.test('it works for n=0', t => {
    const n = 0;
    const m = fromBinary64(toBinary64(n));
    t.equal(n, m);
    t.end();
  });

  t.test('it works for n=3', t => {
    const n = 3;
    const m = fromBinary64(toBinary64(n));
    t.equal(n, m);
    t.end();
  });

  t.test('it works for n=performance.now()', t => {
    const n = performance.now();
    const m = fromBinary64(toBinary64(n));
    t.equal(n, m);
    t.end();
  });
});

/**
 * Convert a byte to 8 bits.
 * @param {number} byte - a byte
 * @return {Array<boolean>} bits - 8 bits
 */
function byteToBits(byte) {
  return [
    !!((1 << 7) & byte),
    !!((1 << 6) & byte),
    !!((1 << 5) & byte),
    !!((1 << 4) & byte),
    !!((1 << 3) & byte),
    !!((1 << 2) & byte),
    !!((1 << 1) & byte),
    !!((1 << 0) & byte)
  ];
}

tape('byteToBits(byte)', t => {
  t.test('it works for byte=0', t => {
    const bits = byteToBits(0);
    t.ok(bits.length, 'returns an 8-bit bitstring');
    t.ok(bits.every(bit => bit === false), 'every bit is false');
    t.end();
  });

  t.test('it works for byte=3', t => {
    const bits = byteToBits(3);
    t.deepEqual(bits, [
      false, false, false, false,
      false, false,  true,  true
    ]);
    t.end();
  });

  t.test('it works for byte=4', t => {
    const bits = byteToBits(4);
    t.deepEqual(bits, [
      false, false, false, false,
      false,  true, false, false
    ]);
    t.end();
  });
});

/**
 * Convert 8 bits to a byte
 * @param {Array<bits>} bits - 8 bits
 * @returns {number} byte - a byte
 */
function bitsToByte(bits) {
  if (bits.length !== 8) {
    throw new Error('Expected 8 bits');
  }
  return bits.reduce((byte, bit, i) => {
    return byte | bit << 8 - i - 1;
  }, 0);
}

tape('bitsToBytes(bits)', t => {
  t.test('it works for bits=00000000', t => {
    const result = bitsToByte([
      false, false, false, false,
      false, false, false, false
    ]);
    t.equal(result, 0);
    t.end();
  });

  t.test('it works for bits=00000011', t => {
    const result = bitsToByte([
      false, false, false, false,
      false, false,  true, true
    ]);
    t.equal(result, 3);
    t.end();
  });

  t.test('it works for bits=00000100', t => {
    const result = bitsToByte([
      false, false, false, false,
      false,  true, false, false
    ]);
    t.equal(result, 4);
    t.end();
  });
});

function flatMap(xs, f) {
  return xs.reduce((ys, x) => ys.concat(f(x)), []);
}

tape('flatMap(xs, f)', t => {
  t.test('it works', t => {
    const result = flatMap([1, 2, 3, 4], x => [x, -x]);
    t.deepEqual(result, [
      1, -1,
      2, -2,
      3, -3,
      4, -4
    ]);
    t.end();
  });
});

function chunks(xs, n) {
  if (n < 0) {
    throw new Error('n must be non-negative');
  }
  return xs.length <= n
    ? [xs]
    : [xs.slice(0, n)].concat(chunks(xs.slice(n), n));
}

tape('chunks(xs, n)', t => {
  t.test('it works', t => {
    const result = chunks([1, 2, 3, 4, 5, 6], 2);
    t.deepEqual(result, [
      [1, 2],
      [3, 4],
      [5, 6]
    ]);
    t.end();
  });
});

function repeat(n, x) {
  const xs = new Array(n);
  for (let i = 0; i < n; i++) {
    xs[i] = x;
  }
  return xs;
}

/**
 * Convert a non-negative number to its bits.
 * @param {number} n - the number
 * @returns {Array<bool>} bits - 64 bits
 */
function toBits(n) {
  if (n < 0) {
    throw new Error('n must be non-negative');
  }
  const bits = n.toString(2).split('').map(x => x === '1');
  const zeroes = repeat(64 - bits.length, false);
  return zeroes.concat(bits);
}

tape('toBits(n)', t => {
  t.test('it works for n=0', t => {
    const bits = toBits(0);
    t.equal(bits.length, 64, 'returns a 64-bit bitstring');
    t.ok(bits.every(bit => bit === false), 'every bit is false');
    t.end();
  });

  t.test('it works for n=3', t => {
    const bits = toBits(3);
    t.ok(bits.slice(0, 62).every(bit => bit === false), 'the first 62 bits are false');
    t.ok(bits.slice(62).every(bit => bit === true), 'the last two bits are true');
    t.end();
  });
});

/**
 * Convert bits to a non-negative number.
 * @param {Array<bool>} bits - 64 bits
 * @returns {number} n - the non-negative number
 */
function fromBits(bits) {
  if (bits.length !== 64) {
    throw new Error('Expected 64 bits');
  }
  return Number.parseInt(bits.map(bit => bit ? '1' : '0').join(''), 2);
}

tape('fromBits(toBits(n))', t => {
  t.test('it works for n=0', t => {
    const n = 0;
    const m = fromBits(toBits(n));
    t.equal(n, m);
    t.end();
  });

  t.test('it works for n=3', t => {
    const n = 3;
    const m = fromBits(toBits(n));
    t.equal(n, m);
    t.end();
  });

  t.test('it works for n=Date.now()', t => {
    const n = Date.now();
    const m = fromBits(toBits(n));
    t.equal(n, m);
    t.end();
  });
});

exports.chunks = chunks;
exports.flatMap = flatMap;
exports.fromBinary64 = fromBinary64;
exports.fromBits = fromBits;
exports.toBinary64 = toBinary64;
exports.toBits = toBits;
