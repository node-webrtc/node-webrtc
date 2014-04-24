#!/usr/bin/env node
(function() {
  'use strict';

  var os = require('os');
  var fs = require('fs');
  var spawn = require('child_process').spawn;
  var nopt = require('nopt');

  var PROJECT_DIR = process.cwd();
  var DEPOT_TOOLS_REPO = 'https://chromium.googlesource.com/chromium/tools/depot_tools.git';
  var LIB_WEBRTC_DIR_REPO = 'http://webrtc.googlecode.com/svn/trunk';
  var LIB_DIR = PROJECT_DIR + '/third_party';
  var LIB_WEBRTC_DIR = LIB_DIR + '/libwebrtc';
  var DEPOT_TOOLS_DIR = LIB_DIR + '/depot_tools';
  var GCLIENT = 'gclient';
  var NINJA = 'ninja';
  var MAKE = 'make';
  var NODE_GYP = PROJECT_DIR + '/node_modules/.bin/node-gyp';

  var knownOpts = {
    'target-arch': String,
    'gyp-gen': String,
    'verbose': Boolean,
    'libwebrtc-revision': String,
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
  var LIBWEBRTC_REVISION = parsed['libwebrtc-revision'] || 'r5459';
  var CONFIGURATION = parsed['configuration'] || 'Release'
  //var CONFIGURATION = 'Release';

  console.log("TARGET_ARCH="+TARGET_ARCH, LIBWEBRTC_REVISION, CONFIGURATION);

  process.env.PATH = DEPOT_TOOLS_DIR + ':' + process.env.PATH;
  process.env.GYP_GENERATORS = NINJA;
  process.env.GYP_DEFINES = ('host_arch=' + HOST_ARCH + ' target_arch=' + TARGET_ARCH);

  function clone_depot_tools() {
    process.stdout.write('Cloning depot tools ... ');
    if(!fs.existsSync(DEPOT_TOOLS_DIR)) {
      var proc = spawn('git',
        ['-C', LIB_DIR, 'clone', '-v', '--progress', DEPOT_TOOLS_REPO]
      );
      var log = fs.createWriteStream(PROJECT_DIR+'/build.log', {flags: 'w'});
      proc.stdout.pipe(log);
      proc.stderr.pipe(log);
      proc.on('exit', function(code, signal) {
        if(undefined !== code && 0 !== code) {
          process.stderr.write('error (see build.log for details): ', code, signal, '\r\n');
          process.exit(-1);
        } else {
          proc.stdout.unpipe(log);
          proc.stderr.unpipe(log);
          process.stdout.write('done\r\n');
          process.nextTick(gclient_config);
        }
      });
    } else {
      process.stdout.write('skip\r\n');
      process.nextTick(gclient_config);
    }
  }

  function gclient_config() {
    if(!fs.existsSync(LIB_WEBRTC_DIR)) {
      if(!fs.existsSync(LIB_DIR))
        fs.mkdirSync(LIB_DIR);
      fs.mkdirSync(LIB_WEBRTC_DIR);
    }

    process.stdout.write('Configuring gclient ... ');
    process.chdir(LIB_WEBRTC_DIR);
    var proc = spawn(GCLIENT,
      ['config', LIB_WEBRTC_DIR_REPO]
    );
    var log = fs.createWriteStream(PROJECT_DIR+'/build.log', {flags: 'a'});
    proc.stdout.pipe(log);
    proc.stderr.pipe(log);
    proc.on('exit', function(code, signal) {
      if(undefined !== code && 0 !== code) {
        process.stderr.write('error (see build.log for details): ', code, signal, '\r\n');
        process.exit(-1);
      } else {
        proc.stdout.unpipe(log);
        proc.stderr.unpipe(log);
        process.stdout.write('done\r\n');
        process.nextTick(gclient_sync);
      }
    });
  }

  function gclient_sync() {
    process.stdout.write('Syncing upstream libjingle ... ');
    process.chdir(LIB_WEBRTC_DIR);
    var proc = spawn(GCLIENT,
      ['sync', '-f', '-n', '-D', '-j1', '-r'+LIBWEBRTC_REVISION]
    );
    var log = fs.createWriteStream(PROJECT_DIR+'/build.log', {flags: 'a'});
    proc.stdout.pipe(log);
    proc.stderr.pipe(log);
    proc.on('exit', function(code, signal) {
      if(undefined !== code && 0 !== code) {
        process.stderr.write('error (see build.log for details): ', code, signal, '\r\n');
        process.exit(-1);
      } else {
        proc.stdout.unpipe(log);
        proc.stderr.unpipe(log);
        process.stdout.write('done\r\n');
        process.nextTick(gclient_runhooks);
      }
    });
  }

  function gclient_runhooks(cb) {
    process.stdout.write('Executing runhooks ... ');
    process.chdir(LIB_WEBRTC_DIR);
    var proc = spawn(GCLIENT,
      ['runhooks', '-j1']
    );
    var log = fs.createWriteStream(PROJECT_DIR+'/build.log', {flags: 'a'});
    proc.stdout.pipe(log);
    proc.stderr.pipe(log);
    proc.on('exit', function(code, signal) {
      if(undefined !== code && 0 !== code) {
        process.stderr.write('error (see build.log for details): ', code, signal, '\r\n');
        process.exit(-1);
      } else {
        proc.stdout.unpipe(log);
        proc.stderr.unpipe(log);
        process.stdout.write('done\r\n');
        process.nextTick(build);
      }
    });
  }

  function build() {
    process.stdout.write('Building libjingle ... ');
    var args = ['-C', 'trunk/out/' + CONFIGURATION];

    switch(PLATFORM) {
      case 'linux':
        args.push('peerconnection_client');
        break;
      default:
        break;
    }

    process.chdir(LIB_WEBRTC_DIR);
    var proc = spawn(NINJA,
      args
    );
    var log = fs.createWriteStream(PROJECT_DIR+'/build.log', {flags: 'a'});
    proc.stdout.pipe(log);
    proc.stderr.pipe(log);
    proc.on('exit', function(code, signal) {
      if(undefined !== code && 0 !== code) {
        process.stderr.write('error (see build.log for details): ', code, signal, '\r\n');
        process.exit(-1);
      } else {
        proc.stdout.unpipe(log);
        proc.stderr.unpipe(log);
        process.stdout.write('done\r\n');
        process.nextTick(complete);
      }
    });
  }

  function complete() {
    process.stdout.write('Build complete\r\n');
  }

  if(process.env.WRTC_SKIP_SYNC) {
    build();
  } else {
    clone_depot_tools();
  }

/*
  function processOutput(spawnedProcess, processType) {
    return new RSVP.Promise(function(resolve, reject) {
      spawnedProcess.stdout.on('data',
        function(data) {
          if (VERBOSE) {
            process.stdout.write(data);
          } else {
            process.stdout.write('.');
          }
        }
      ).on('error',
        function(error) {
          if (VERBOSE) {
            process.stdout.write(error);
          } else {
            process.stdout.write('[failure]');
          }
        }
      ).on('end',
        function() {
          if (VERBOSE) {
            process.stdout.write('\r\n' + processType + ' finished\r\n');
          } else {
            process.stdout.write('.\r\n');
          }
        });

      spawnedProcess.stderr.on('data',
        function(data) {
          if (VERBOSE) {
            process.stdout.write(data);
          } else {
            process.stdout.write('.');
          }
        }
      );

      spawnedProcess.on('exit',
        function(code, signal) {
          if (code !== undefined &&
            code !== 0) {
            reject('For ' + processType + ' something went wrong: error code '+ code + ', signal ' + signal + '.\r\n\tTry to run build script with --verbose option to find the possible problem.\r\n');
          } else {
            resolve();
          }
        }
      );
    });
  }

  function downloadDepotTools() {
    return new RSVP.Promise(function(resolve, reject) {

      process.stdout.write('Going to eventually download depot tools from ' +
        DEPOT_TOOLS_REPO + ' in ' + TOOLS_DEPOT_TOOLS_DIR + '\r\n');

      if (!fs.existsSync(TOOLS_DIR)) {

        fs.mkdirSync(TOOLS_DIR);
      }

      process.chdir(TOOLS_DIR);
      if(!fs.existsSync(TOOLS_DEPOT_TOOLS_DIR)) {

        var gitCloneDepotTools = spawn('git', [
          'clone', '-v', '--progress', DEPOT_TOOLS_REPO
        ]);

        processOutput(gitCloneDepotTools, 'cloning depot tools').then(function(){

          resolve();
        }, function(rejectionInfo) {

          reject(rejectionInfo);
        });
      } else {

        if (VERBOSE) {

          process.stdout.write('You already have depot tools, going on...\r\n');
        } else {

          process.stdout.write('.');
        }
        resolve();
      }
    });
  }

 function runGclientConfig() {
    return new RSVP.Promise(function(resolve, reject) {

      process.stdout.write('Going to run the depot tools command: ' +
        GCLIENT + ' config ' + LIB_WEBRTC_DIR_REPO + ' in folder ' + LIB_WEBRTC_DIR + ' \r\n');

      if(!fs.existsSync(LIB_WEBRTC_DIR)) {

        if(!fs.existsSync(LIB_DIR)) {

          fs.mkdirSync(LIB_DIR);
        }

        fs.mkdirSync(LIB_WEBRTC_DIR);
      }
      process.chdir(LIB_WEBRTC_DIR);

      var gclientConfig = spawn(GCLIENT, [
        'config',
        LIB_WEBRTC_DIR_REPO
      ]);

      var processName = GCLIENT + ' config ' + LIB_WEBRTC_DIR_REPO;
      processOutput(gclientConfig, processName).then(function(){

          resolve();
        }, function(rejectionInfo) {

          reject(rejectionInfo);
        });
    });
  }

function runGclientSync() {
    return new RSVP.Promise(function(resolve, reject) {

      process.stdout.write('Going to run the depot tools command: '+
        GCLIENT + ' sync -D --verbose -j1 in folder '+ LIB_WEBRTC_DIR + '\r\n');

      process.chdir(LIB_WEBRTC_DIR);
      var gclientSync = spawn(GCLIENT, ['sync', '-D', '--verbose', '-j1']);

      var processName = GCLIENT + ' sync -D --verbose -j1';
      processOutput(gclientSync, processName).then(function(){

          resolve();
        }, function(rejectionInfo) {

          reject(rejectionInfo);
        });
    });
  }

 function runGlientRunHooks() {
    return new RSVP.Promise(function(resolve, reject) {
0
      process.stdout.write('Going to run the depot tools command: '+
        GCLIENT + ' runhooks in folder '+ LIB_WEBRTC_DIR + '\r\n');

      process.chdir(LIB_WEBRTC_DIR);
      var gclientRunHooks = spawn(GCLIENT, ['runhooks', '--force']);

      var processName = GCLIENT + ' runhooks';
      processOutput(gclientRunHooks, processName).then(function(){

          resolve();
        }, function(rejectionInfo) {

          reject(rejectionInfo);
        });
    });
  }

 function runNinja() {
    return new RSVP.Promise(function(resolve, reject) {

      process.stdout.write('Going to run the depot tools command: ' + NINJA +
        ' -C trunk/out/Release in folder '+ LIB_WEBRTC_DIR + '\r\n');

      var ninjaArguments = [];
      ninjaArguments.push('-C');
      ninjaArguments.push('trunk/out/Release');
      if('linux' === os.platform()) {

        ninjaArguments.push('peerconnection_client');
      }

      process.chdir(LIB_WEBRTC_DIR);
      var make = spawn(NINJA, ninjaArguments);

      var processName = NINJA;
      processOutput(make, processName).then(function(){

          resolve();
        }, function(rejectionInfo) {

          reject(rejectionInfo);
        });
    });
  }

  downloadDepotTools().then(
    function() {
      return runGclientConfig();
    }
  ).then(
    function() {
      return runGclientSync();
    }
  ).then(
    function() {
      return runGlientRunHooks();
    }
  ).then(
    function() {
      return runNinja();
    }
  ).then(
    function() {
      process.stdout.write('wrtc module build complete\r\n');
      process.exit(0);
    }
  ).catch(
    function(rejectionInfo) {
      process.stderr.write(rejectionInfo);
      process.exit(1);
    }
  );
*/
})();
