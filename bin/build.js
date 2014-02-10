#!/usr/bin/sh

var async = require('async');
var path = require('path');
var os = require('os');
var fs = require('fs');
var sys = require('sys');
var mkdirp = require('mkdirp');
var exec = require('child_process').exec;
var spawn = require('child_process').spawn;
var gclient = require('depot_tools/gclient');
var ninja = require('depot_tools/ninja');
var PROJECT_DIR = path.resolve(__dirname, '..');
var LIB_WEBRTC_DIR = path.join(PROJECT_DIR, 'third_party', 'libwebrtc');

var tasks = [
  ['config', 'http://webrtc.googlecode.com/svn/trunk'],
  ['sync'],
  ['runhooks']
].map(gclient(LIB_WEBRTC_DIR)).concat([
  ninja(LIB_WEBRTC_DIR, ['-C', 'trunk/out/Release']),
  nodegyp_build
]);

mkdirp(LIB_WEBRTC_DIR, function(err) {
  async.series(tasks, function(err) {
    console.log(err ? 'Build errored: ' + err : 'Build complete');
  });
});

function read_stream(stream)
{
  stream.read();
  process.stdout.write('.');
}

function nodegyp_build(callback)
{
  console.info("nodegyp_build");
  process.stdout.write('.');

  process.chdir(PROJECT_DIR);

  var child = spawn("node-gyp", ["rebuild"]);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal)
  {
    process.stdout.write('\n');
    if(0 === code && !signal)
    {
      console.log('nodegyp_build complete');
      callback();
    }
    else
    {
      console.error('nodegyp_build failed:', code, signal);
    }
  });
}

// depot_tools(function()
// {
//   console.log('wrtc build complete');
// });
