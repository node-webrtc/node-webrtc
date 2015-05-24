#!/usr/bin/env node
(function() {
  'use strict';

  var os = require('os');
  var fs = require('fs-extra');
  var Path = require('path');
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
  var PYTHON = process.env['PYTHON'] || 'python';
  var NODE_GYP = PROJECT_DIR + '/node_modules/.bin/node-gyp';
  var BUILD_DIR = Path.join(__dirname, '../build');

  var knownOpts = {
    'target-arch': String,
    'gyp-gen': String,
    'verbose': Boolean,
    'configuration': String,
    'module_path': String,
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
  var MODULE_PATH = parsed['module_path'];
  var CONFIGURATION = parsed['configuration'] || 'Release';
  var IS_DEBUG = CONFIGURATION == 'Debug';
  var symlinks = [];

  console.log("TARGET_ARCH="+TARGET_ARCH, "PLATFORM="+PLATFORM, "CONFIGURATION="+CONFIGURATION, "PYTHON="+PYTHON, "MODULE_PATH="+MODULE_PATH);

  process.env.PATH = DEPOT_TOOLS_DIR + ':' + process.env.PATH;
  process.env.GYP_GENERATORS = NINJA;
  process.env.GYP_DEFINES = ('host_arch=' + HOST_ARCH + ' target_arch=' + TARGET_ARCH);

  function spawn_log(cmd, args, opts, nextstep) {
    if (arguments.length == 3) {
        nextstep = opts;
        opts = {};
    }


    opts.stdio = ['ignore', process.stdout, process.stderr];
    var proc = spawn(cmd, args, opts);
    proc.on('exit', function(code, signal) {
      if (code !== undefined && code !== 0) {
        console.error('ERROR:', code, signal, '\n');
        process.exit(-1);
      } else {
        process.nextTick(nextstep);
      }
    });
  }

  function prepare_directories() {
    if(!fs.existsSync(LIB_DIR)) {
      console.log(': Preparing directories ... ');
      fs.mkdirSync(LIB_DIR);
    }

    process.nextTick(clone_depot_tools);
  }

  function clone_depot_tools() {
    var next = clone_libwebrtc_repo;

    if(!fs.existsSync(DEPOT_TOOLS_DIR)) {
      console.log(': Cloning depot_tools ... ');
      spawn_log('git',
        ['clone', '--depth', '1', '-v', '--progress', DEPOT_TOOLS_REPO],
        {'cwd': LIB_DIR},
        next
      );
    } else {
      console.log(': Updating depot_tools ... ');
      spawn_log('git',
        ['pull', 'origin', 'master'],
        {'cwd': DEPOT_TOOLS_DIR},
        next
      );
    }
  }

  function clone_libwebrtc_repo() {
    var next = PLATFORM == 'win32' ? win_generate_symlinks : update_clang;

    if(!fs.existsSync(LIB_WEBRTC_DIR)) {
      console.log(': Cloning libwebrtc ... ');
      spawn_log('git',
        ['clone', '--depth', '1', '-v', '--progress', LIB_WEBRTC_REPO],
        {'cwd': LIB_DIR},
        next
      );
    } else {
      console.log(': Updating libwebrtc ... ');
      spawn_log('git',
        ['pull', 'origin', 'master'],
        {'cwd': LIB_WEBRTC_DIR},
        next
      );
    }
  }

  function update_clang() {
    var CLANG_SCRIPT_DIR = LIB_WEBRTC_DIR + '/chromium/src/tools/clang/scripts';
    console.log(': Updating clang ... ');
    spawn_log('bash',
      ['update.sh'],
      {'cwd': CLANG_SCRIPT_DIR},
      generate_build_scripts);
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

  function git_win_process_symlink(linkname, cb) {
    var checkoutProc = spawn('git', ['checkout', '--', linkname]);
    checkoutProc.on('exit', function (code, signal) {
      var flag;
      var dest;
      var stats;
      var fullpath;
      var linkProc;

      if (code !== undefined && code !== 0) {
        console.error('ERROR:', code, signal, '\n');
        cb();
      } else {
        dest = fs.readFileSync(linkname, 'utf-8');
        fullpath = Path.dirname(linkname) + '/' + dest;

        try {
          stats = fs.statSync(fullpath);
        } catch (e) {
          cb();
          return;
        }

        if (stats.isFile()) {
          flag = '/H';
        } else if (stats.isDirectory()) {
          flag = '/J';
        } else {
          cb();
          return;
        }

        fs.unlinkSync(linkname);
        linkProc = spawn('cmd', ['/C', 'mklink', flag, linkname.replace(/\//g, "\\"), fullpath.replace(/\//g, "\\") ]);
        linkProc.on('exit', function (code, signal) {
          if (code !== undefined && code !== 0) {
            console.error('ERROR:', code, signal, '\n');
          }
          cb();
        });
      }
    });
  }

  function git_win_process_symlinks() {
    var i = 0;
    var l = symlinks.length;

    console.log(': Processing symbolic links ... ');
    git_win_process_symlink(symlinks[i++], function git_win_process_symlink_cb() {
      if (i < l) {
        git_win_process_symlink(symlinks[i++], git_win_process_symlink_cb);
      } else {
        generate_build_scripts();
      }
    });
  }

  function win_generate_symlinks() {
    var lsProc;
    stopTimer();

    console.log(': Fetching symbolic links ... ');
    process.chdir(LIB_WEBRTC_DIR);
    lsProc = spawn('git', ['ls-files', '-s']);

    lsProc.stdout.on('data', function(data) {
      var i = 0;
      var str = data.toString();
      var lines = str.split(/(\r?\n)/g);
      var l = lines.length;

      for (var i = 0, l = lines.length; i < l; ++i) {
        var line = lines[i];
        if (line && line.match(/^120000/)) {
          var tab = line.split(/[\s]+/);
          if (tab && tab.length == 4) {
            try {
              fs.accessSync(tab[3], fs.R_OK | fs.W_OK);
            } catch (err) {
              continue;
            }
            symlinks.push(tab[3]);
          }
        }
      }
    });

    lsProc.on('exit', function (code, signal) {
      if (code !== undefined && code !== 0) {
        console.error('ERROR:', code, signal, '\n');
      } else {
        git_win_process_symlinks();
      }
    });

    startTimer();
  }

  function generate_build_scripts() {
    stopTimer();

    console.log(': Generating build scripts ... ');
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

    console.log(': Building libwebrtc ... ');
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

    fs.writeFileSync(Path.join(BUILD_DIR, 'wrtc.json'), JSON.stringify({debug: IS_DEBUG}));
    console.log(': Build complete\n');
  }

  prepare_directories();

})();
