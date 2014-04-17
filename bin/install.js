#!/usr/bin/env node

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
    'gyp-gen': String,
  };

  var shortHands = {
    'd': '--debug',
    'f': '--force',
    't': '--target-arch',
    'x86': ['--target-arch', 'x86'],
    'x64': ['--target-arch', 'x64'],
    'arm': ['--target-arch', 'arm'],
    'ninja': ['--gyp-gen', 'ninja'],
  };

  var parsed = nopt(knownOpts, shortHands, process.argv, 2);

  var DEBUG = parsed['debug'] || false;
  var FORCE = parsed['force'] || false;
  var TARGET_ARCH = parsed['target-arch'] || process.arch;
  var HOST_ARCH = process.arch;
  var KNOWN_ARCH = ['x86', 'x64'];
  var PLATFORM = process.platform;
  var V8 = /[0-9]+\.[0-9]+/.exec(process.versions.v8)[0];

  console.log(HOST_ARCH, TARGET_ARCH, PLATFORM, V8, "debug:" + DEBUG, "force:" + FORCE);

  if(KNOWN_ARCH.indexOf(TARGET_ARCH) < 0) {
    console.error('Unsupported target architecture: ' + TARGET_ARCH);
    process.exit(-1);
  }

  // var modPath = [PLATFORM, ARCH, 'v8-' + V8].join('_');

  function check() {
    try {
        var webrtc = require('bindings')('webrtc.node');
        return true;
      } catch(e) {
        return false;
      }
  }

  function fetch() {
    return false;
  }

  function build() {
    var PROJECT_DIR = process.cwd();
    var DEPOT_TOOLS_REPO = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git';
    var LIB_WEBRTC_DIR_REPO = 'http://webrtc.googlecode.com/svn/trunk';
    var LIB_DIR = PROJECT_DIR + '/third_party';
    var LIB_WEBRTC_DIR = LIB_DIR + '/libwebrtc';
    var DEPOT_TOOLS_DIR = LIB_DIR + '/depot_tools';
    var GCLIENT = 'gclient';
    var NINJA = 'ninja';
    var NODE_GYP = PROJECT_DIR + '/node_modules/.bin/node-gyp';
    var GIT = 'git';

    process.env.PATH = DEPOT_TOOLS_DIR + ':' + process.env.PATH;
    process.env.GYP_GENERATORS = NINJA;
    process.env.GYP_DEFINES = ('host_arch' + HOST_ARCH + 'target_arch' + TARGET_ARCH);

    function update_depot_tools() {
      if (!fs.existsSync(DEPOT_TOOLS_DIR)) {
        var proc__clone_depot_tools = spawn(GIT, 'clone', '--progress', DEPOT_TOOLS_REPO, DEPOT_TOOLS_DIR);
      }
    }

    return false;
  }

  if(!FORCE) {
    var ok;

    console.log('checking for existing module');
    ok = check();

    if(ok) {
      console.log('module found, exiting');
      process.exit(0);
    }

    console.error('module not found, fetching compiled module from upstream');
    ok = fetch();

    if(ok) {
      console.log('compiled module found, exiting');
      process.exit(0);
    }

    console.error('compiled module not found, building');
    ok = build();

    if(ok) {
      console.log('build complete, exiting');
      process.exit(0);
    } else {
      console.error('build failed, see build.log for details');
      process.exit(-1);
    }
  } else {
    var ok;

    console.log('building module from souce');
    ok = build();

    if(ok) {
      console.log('build complete, exiting');
      process.exit(0);
    } else {
      console.error('build failed, see build.log for details');
      process.exit(-1);
    }
  }
})();