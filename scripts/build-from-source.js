#!/usr/bin/env node
/* eslint no-console:0, no-process-env:0, no-process-exit:0 */
'use strict';

const { spawnSync } = require('child_process');

const args = ['configure'];

if (process.env.DEBUG) {
  args.push('--debug');
}

if (process.platform === 'win32') {
  args.push('-g');
  args.push(process.arch === 'x64'
    ? '"Visual Studio 15 2017 Win64"'
    : '"Visual Studio 15 2017"');
}

function main() {
  console.log('Running ncmake ' + args.join(' '));
  let { status } = spawnSync('ncmake', args, {
    shell: true,
    stdio: 'inherit'
  });
  if (status) {
    throw new Error('ncmake configure failed for wrtc');
  }

  console.log('Running ncmake build');
  status = spawnSync('ncmake', ['build'], {
    shell: true,
    stdio: 'inherit'
  }).status;
  if (status) {
    throw new Error('ncmake build failed for wrtc');
  }

  console.log('Built wrtc');
}

main();
