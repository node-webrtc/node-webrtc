'use strict';

var find = require('node-pre-gyp').find;
var path = require('path');
var modulePath = find(path.join(__dirname, '../package.json'));

var binding;
try {
  binding = require(modulePath);
} catch (error) {
  // NOTE(mroberts): Janky, I know.
  binding = require(modulePath.replace(/Release/, 'Debug'));
}

module.exports = binding;
