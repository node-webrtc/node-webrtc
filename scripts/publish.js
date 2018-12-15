#!/usr/bin/env node
/* eslint no-console:0, no-process-env:0 */
'use strict';

var spawnSync = require('child_process').spawnSync;

var args = ['package', 'publish'];

if (process.env.DEBUG === 'true') {
  args = args.concat(['--debug']);
}

if (process.env.TARGET_ARCH) {
  args = args.concat(['--target_arch=' + process.env.TARGET_ARCH]);
}

var result = spawnSync('node-pre-gyp', args, {
  shell: true,
  stdio: 'inherit'
});

if (result.status) {
  throw new Error('Unable to publish wrtc binary');
}

console.log('Published wrtc binary');
