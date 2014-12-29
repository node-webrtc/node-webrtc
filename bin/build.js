#!/usr/bin/env node
(function() {
  'use strict';

  var os = require('os');
  var fs = require('fs');
  var spawn = require('child_process').spawn;
  var nopt = require('nopt');

  var PROJECT_DIR = process.cwd();
  var DEPOT_TOOLS_REPO = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git';
  var LIB_WEBRTC_REPO = 'https://github.com/js-platform/libwebrtc.git';
  var LIB_DIR = PROJECT_DIR + '/third_party';
  var DEPOT_TOOLS_DIR = LIB_DIR + '/depot_tools';
  var LIB_WEBRTC_DIR = LIB_DIR + '/libwebrtc';
  var NINJA = 'ninja';
  var MAKE = 'make';
  var PYTHON = 'python';
  var NODE_GYP = PROJECT_DIR + '/node_modules/.bin/node-gyp';

  var knownOpts = {
    'target-arch': String,
    'gyp-gen': String,
    'verbose': Boolean,
    'configuration': String,
  };

  var shortHands = {
    't': '--target-arch',
    'ia32': ['--target-arch', 'ia32'],
    'x64': ['--target-arch', 'x64'],
    'arm': ['--target-arch', 'arm'],
    'ninja': ['--gyp-gen', 'ninja'],
    'v': '--verbose',
  };

  var parsed = nopt(knownOpts, shortHands, process.argv, 2);

  var TARGET_ARCH = parsed['target-arch'] || process.arch;
  var HOST_ARCH = process.arch;
  var VERBOSE = !!parsed['verbose'];
  var PLATFORM = process.platform;
  var CONFIGURATION = parsed['configuration'] || 'Release'

  console.log("TARGET_ARCH="+TARGET_ARCH, CONFIGURATION);

  process.env.PATH = DEPOT_TOOLS_DIR + ':' + process.env.PATH;
  process.env.GYP_GENERATORS = NINJA;
  process.env.GYP_DEFINES = ('host_arch=' + HOST_ARCH + ' target_arch=' + TARGET_ARCH);

  var first = true;
  function spawn_log(cmd, args, opts, nextstep) {
    if (arguments.length == 3) {
        nextstep = opts;
        opts = {};
    }

    var logpath = PROJECT_DIR + '/build.log';
    if(first && fs.existsSync(logpath)) {
      fs.truncateSync(logpath, 0);
    }
    first = false;

    var log1 = fs.createWriteStream(logpath, {flags: 'a'});
    var log2 = fs.createWriteStream(logpath, {flags: 'a'});

    log1.on('open', function(){
    log2.on('open', function(){
      opts.stdio = ['ignore', log1, log2];
      var proc = spawn(cmd, args, opts);
      proc.on('exit', function(code, signal) {
        if (code !== undefined && code !== 0) {
          process.stderr.write('error (see build.log for details): ', code, signal, '\n');
          process.exit(-1);
        } else {
          process.stdout.write(' done\n');
          process.nextTick(nextstep);
        }
      });
    });
    });
  }

  function prepare_directories() {
    process.stdout.write('Preparing directories ... ');
    if(!fs.existsSync(LIB_DIR)) {
      fs.mkdirSync(LIB_DIR);
    }

    process.stdout.write('done\n');
    process.nextTick(clone_depot_tools);
  }

  function clone_depot_tools() {
    process.stdout.write('Cloning depot tools ... ');
    if(!fs.existsSync(DEPOT_TOOLS_DIR)) {
      spawn_log('git',
        ['clone', '--depth', '1', '-v', '--progress', DEPOT_TOOLS_REPO],
        {'cwd': LIB_DIR},
        clone_libwebrtc_repo
      );
    } else {
      spawn_log('git',
        ['pull'],
        {'cwd': DEPOT_TOOLS_DIR},
        clone_libwebrtc_repo
      );
    }
  }

  function clone_libwebrtc_repo() {
    process.stdout.write('Cloning libwebrtc repository ... ');
    if(!fs.existsSync(LIB_WEBRTC_DIR)) {
      spawn_log('git',
        ['clone', '--depth', '1', '-v', '--progress', LIB_WEBRTC_REPO],
        {'cwd': LIB_DIR},
        generate_build_scripts
      );
    } else {
      spawn_log('git',
        ['pull'],
        {'cwd': LIB_WEBRTC_DIR},
        generate_build_scripts
      );
    }
  }

  var timer = null;

  function startTimer() {
    timer = setInterval(function() {
      process.stdout.write('.');
    }, 10000);
  }

  function stopTimer() {
    if(!timer) return;
    clearInterval(timer);
    timer = null;
  }

  function generate_build_scripts() {
    stopTimer();

    process.stdout.write('Generating build scripts ... ');
    var args = ['webrtc/build/gyp_webrtc'];

    process.chdir(LIB_WEBRTC_DIR);
    spawn_log(PYTHON,
      args,
      build
    );

    startTimer();
  }

  function build() {
    stopTimer();

    process.stdout.write('Building libwebrtc ... ');
    var args = ['-C', 'out/' + CONFIGURATION, 'wrtc_build'];

    process.chdir(LIB_WEBRTC_DIR);
    spawn_log(NINJA,
      args,
      complete
    );

    startTimer();
  }

  function complete() {
    stopTimer();

    process.stdout.write('Build complete\n');
  }

  prepare_directories();

})();
