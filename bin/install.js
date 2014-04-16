#!/usr/bin/env node

function sanity_test(webrtc) {

}

(function() {
  'use strict';

  var os = require('os');
  var fs = require('fs');
  var spawn = require('child_process').spawn;
  var nopt = require('nopt');
  var path = require('path');

  var knownOpts = {
    'debug': Boolean,
    'force': Boolean,
    'target-arch': String,
  };

  var shortHands = {
    'd': '--debug',
    'f': '--force',
    't': '--target-arch',
    'x86': ['--target-arch', 'x86'],
    'x64': ['--target-arch', 'x64'],
    'arm': ['--target-arch', 'arm'],
  };

  var parsed = nopt(knownOpts, shortHands, process.argv, 2);

  var DEBUG = parsed['debug'] || false;
  var FORCE = parsed['force'] || false;
  var ARCH = parsed['target-arch'] || process.arch;
  var KNOWN_ARCH = ['x86', 'x64'];
  var PLATFORM = process.platform;
  var V8 = /[0-9]+\.[0-9]+/.exec(process.versions.v8)[0];

  console.log(ARCH, PLATFORM, V8, DEBUG, FORCE);

  if(KNOWN_ARCH.indexOf(ARCH) < 0) {
    console.error('Unsupported target architecture: ' + ARCH);
    process.exit(-1);
  }

  // var modPath = [PLATFORM, ARCH, 'v8-' + V8].join('_');


  if(!FORCE) {
    (function() {
      try {
        var webrtc = require('bindings')('webrtc.node');
        console.log('module found');
        sanity_check(webrtc);
      } catch(e) {
        console.error('module not found');
      }
    })();
  }
})();