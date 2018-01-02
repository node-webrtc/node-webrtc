#!/usr/bin/env node
/* eslint no-console:0 no-process-env:0 */
'use strict';

var spawnSync = require('child_process').spawnSync;

var args = ['configure'];

if (process.env.DEBUG) {
  args.push('--debug');
}

if (process.platform === 'win32') {
  args.push('-g');
  args.push(process.arch === 'x64'
    ? '"Visual Studio 14 2015 Win64"'
    : '"Visual Studio 14 2015"');
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
  spawnSync('ncmake', args, {
    shell: true,
    stdio: 'inherit'
  });
  console.log('Running ncmake build');
  spawnSync('ncmake', ['build'], {
    shell: true,
    stdio: 'inherit'
  });
  console.log('Built wrtc');
}

if (process.env.SKIP_DOWNLOAD) {
  build();
} else {
  download();
}
