#!/usr/bin/env node
'use strict';
/* eslint no-console:0, no-process-env:0 */

const build = require('./build-from-source');
const download = require('./download-prebuilt');

function main() {
  if (!process.env.SKIP_DOWNLOAD) {
    try {
      console.log('Searching for a pre-built wrtc binary');
      download();
      console.log('Installed a pre-built wrtc binary');
      return;
    } catch (error) {
      console.log('Unable to install a pre-built wrtc binary; falling back to ncmake');
    }
  }
  build();
}

main();
