/* eslint no-process-exit:0 */
'use strict';

const binding = require('../lib/binding');

if (typeof binding.test === 'function') {
  const result = binding.test();
  process.exit(result);
}
