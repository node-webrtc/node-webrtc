var path = require('path');
var os = require('os');
var fs = require('fs');
var sys = require('sys');
var exec = require('child_process').exec;
var spawn = require('child_process').spawn;

var PROJECT_DIR = process.cwd();
var LIB_DIR = PROJECT_DIR + '/third_party';
var LIB_WEBRTC_DIR = LIB_DIR + '/libwebrtc';

function read_stream(stream)
{
  stream.read();
  process.stdout.write('.');
}

var child;

if(!fs.existsSync(LIB_DIR))
{
  fs.mkdirSync(LIB_DIR);
}

if(!fs.existsSync(LIB_WEBRTC_DIR))
{
  fs.mkdirSync(LIB_WEBRTC_DIR);
}

process.env['GYP_GENERATORS'] = 'ninja';

function gclient_config(callback)
{
  console.info("gclient_config");
  process.stdout.write('.');
  process.chdir(LIB_WEBRTC_DIR);
  child = exec("gclient config http://webrtc.googlecode.com/svn/trunk");
  child.on('exit', function(code, signal)
  {
    process.stdout.write('\n');
    if(0 === code && !signal)
    {
      console.log('gclient_config complete');
      gclient_sync(callback);
    }
    else
    {
      console.error('gclient_config failed:', code, signal);
    }
  });
}

function gclient_sync(callback)
{
  console.info("gclient_sync");
  process.stdout.write('.');
  child = spawn("gclient", ["sync"]);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal) {
    process.stdout.write('\n');
    if(0 === code && !signal)
    {
      console.log('gclient_sync complete');
      gclient_runhooks(callback);
    }
    else
    {
      console.error('gclient_sync failed:', code, signal);
    }
  });
}

function gclient_runhooks(callback)
{
  console.info("gclient_runhooks");
  process.stdout.write('.');
  child = spawn("gclient", ["runhooks"]);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal) {
    process.stdout.write('\n');
    if(0 === code && !signal)
    {
      console.log('gclient_runhooks complete');
      ninja_build(callback);
    }
    else
    {
      console.error('gclient_runhooks failed:', code, signal);
    }
  });
}

function ninja_build(callback)
{
  console.info("ninja_build");
  process.stdout.write('.');
  var args = ["-C", "trunk/out/Release"];
  if('linux' == os.platform())
  {
    args.push("peerconnection_client");
  }
  child = spawn("ninja", args);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal) {
    process.stdout.write('\n');
    if(0 === code && !signal)
    {
      console.log('ninja_build complete');
      nodegyp_build(callback);
    }
    else
    {
      console.error('ninja_build failed:', code, signal);
    }
  });
}

function nodegyp_build(callback)
{
  console.info("nodegyp_build");
  process.stdout.write('.');
  process.chdir(PROJECT_DIR);
  child = spawn("node-gyp", ["rebuild"]);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal) {
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

gclient_config(function()
{
  console.log('wrtc build complete');
})

