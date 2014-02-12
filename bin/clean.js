#!/usr/bin/env node
(function() {
  'use strict';
  var spawn = require('child_process').spawn
    , RSVP = require('rsvp');

  var PROJECT_DIR = process.cwd()
    , SRC_DIR = PROJECT_DIR + '/src'
    , NODE_GYP = PROJECT_DIR + '/node_modules/node-gyp/bin/node-gyp.js'
    , VERBOSE = false;

  var argz = process.argv.slice(2);
  argz.include = function(obj) {
    return (this.indexOf(obj) !== -1);
  };

  if(argz.length > 0) {

    if (argz.include('-v') ||
      argz.include('--verbose')) {

      VERBOSE = true;
    }

    if (argz.include('-h') ||
      argz.include('--help')) {

      process.stdout.write('node-webrtc clean script usage:\r\n' +
        '\t-v, --verbose: switch on verbose mode (suggested on build failure);\r\n' +
        '\t-h, --help: print this help.' +
        '\r\n');
      process.exit();
    }
  }

  var processOutput = function(spawnedProcess, processType) {
        return new RSVP.Promise(function(resolve, reject) {
          spawnedProcess.stdout
            .on('data', function(data) {

              if (VERBOSE) {

                process.stdout.write(data);
              } else {

                process.stdout.write('.');
              }
            })
            .on('error', function(error) {

              reject('Error during ' + processType + '\r\n' + error + '\r\n');
            })
            .on('end', function() {

              if (VERBOSE) {

                process.stdout.write('\r\n' + processType + ' finished\r\n');
              } else {

                process.stdout.write('.\r\n');
              }

              resolve();
            });

          spawnedProcess.stderr
            .on('data', function(data) {

              if (VERBOSE) {

                process.stdout.write(data);
              } else {

                process.stdout.write('.');
              }
            });

          spawnedProcess.on('exit', function(code, signal) {
            if (code !== undefined &&
              code !== 0) {

              reject('For ' + processType + ' something went wrong: error code '+ code + ', signal ' + signal + '.\r\n\tTry to run build script with --verbose option to find the possible problem.\r\n');
            }
          });
        });
      }
    , removeAll = function() {
        return new RSVP.Promise(function(resolve, reject) {

          process.stdout.write('Going to remove build, docs, third_party, tools folders.\r\n');

          var remove = spawn('rm', ['-Rf', 'build', 'docs', 'third_party', 'tools']);

          var processName = 'remove all';
          processOutput(remove, processName).then(function(){

              resolve();
            }, function(rejectionInfo) {

              reject(rejectionInfo);
            });
        });
      }
    , runNodeGypClean = function() {
        return new RSVP.Promise(function(resolve, reject) {

          process.stdout.write('Going to run commnad: ' + NODE_GYP + ' clean in folder '+ SRC_DIR + '\r\n');

          process.chdir(SRC_DIR);
          var nodeGyp = spawn(NODE_GYP, ['clean']);

          var processName = NODE_GYP;
          processOutput(nodeGyp, processName).then(function(){

              resolve();
            }, function(rejectionInfo) {

              reject(rejectionInfo);
            });
        });
      };

  removeAll().then(function() {

    return runNodeGypClean();
  }).then(function() {

    process.stdout.write('wrtc module clean complete\r\n');
    process.exit(0);
  }).catch(function(rejectionInfo) {

    process.stderr.write(rejectionInfo);
    process.exit(1);
  });
})();