#!/usr/bin/env node
/* eslint no-console:0, no-process-env:0, no-process-exit:0 */
'use strict';

var spawnSync = require('child_process').spawnSync;

var args = ['configure'];

if (process.env.DEBUG) {
  args.push('--debug');
}

if (process.platform === 'win32') {
  args.push('-g');
  args.push(process.arch === 'x64'
    ? '"Visual Studio 15 2017 Win64"'
    : '"Visual Studio 15 2017"');
}

function download() {
  try {
    console.log('Searching for a pre-built wrtc binary');
    var result = spawnSync('node-pre-gyp', ['install', '--fallback-to-build=false'], {
      shell: true,
      stdio: 'inherit'
    });
    if (result.status) {
      throw new Error('Unable to install a pre-built wrtc binary');
    }
    console.log('Installed a pre-built wrtc binary');
  } catch (error) {
    console.log('Unable to install a pre-built wrtc binary; falling back to ncmake');
    build();
  }
}

function build() {
  console.log('Running ncmake ' + args.join(' '));
  var result1 = spawnSync('ncmake', args, {
    shell: true,
    stdio: 'inherit'
  });
  if (result1.status) {
    throw new Error('ncmake configure failed for wrtc');
  }
  console.log('Running ncmake build');
  var result2 = spawnSync('ncmake', ['build'], {
    shell: true,
    stdio: 'inherit'
  });
  if (result2.status) {
    throw new Error('ncmake build failed for wrtc');
  }
  console.log('Built wrtc');
}

if (process.env.SKIP_DOWNLOAD) {
  build();
} else {
  download();
}
