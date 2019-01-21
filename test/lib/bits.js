'use strict';

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
  const zeroes = '0'.repeat(64 - bits.length).split('').map(() => false);
  return zeroes.concat(bits);
}

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

exports.fromBits = fromBits;
exports.toBits = toBits;
