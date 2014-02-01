#!/usr/bin/sh

var path = require('path');
var os = require('os');
var fs = require('fs');
var sys = require('sys');
var exec = require('child_process').exec;
var spawn = require('child_process').spawn;


var PROJECT_DIR = process.cwd();

var LIB_DIR = PROJECT_DIR + '/third_party';
var LIB_WEBRTC_DIR = LIB_DIR + '/libwebrtc';

var TOOLS_DIR = PROJECT_DIR + '/tools';
var TOOLS_DEPOT_TOOLS_DIR = TOOLS_DIR + '/depot_tools';


var GCLIENT = TOOLS_DEPOT_TOOLS_DIR+"/gclient";


function read_stream(stream)
{
  stream.read();
  process.stdout.write('.');
}


process.env['GYP_GENERATORS'] = 'ninja';


function depot_tools(callback)
{
  console.info("depot_tools");
  process.stdout.write('.');

  // Directory for tools
  if(!fs.existsSync(TOOLS_DIR))
  {
    fs.mkdirSync(TOOLS_DIR);
  }
  process.chdir(TOOLS_DIR);

  // Download depot tools
  if(!fs.existsSync(TOOLS_DEPOT_TOOLS_DIR))
  {
    var child = exec("git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git");
    child.on('exit', function(code, signal)
    {
      process.stdout.write('\n');
      if(0 === code && !signal)
      {
        console.log('depot_tools complete');
        gclient_config(callback);
      }
      else
      {
        console.error('depot_tools failed:', code, signal);
      }
    });
  }
  else
  {
    console.log('depot_tools not required');
    gclient_config(callback);
  }
}

function gclient_config(callback)
{
  console.info("gclient_config");
  process.stdout.write('.');

  // Directory for libwebrtc
  if(!fs.existsSync(LIB_WEBRTC_DIR))
  {
    if(!fs.existsSync(LIB_DIR))
    {
      fs.mkdirSync(LIB_DIR);
    }

    fs.mkdirSync(LIB_WEBRTC_DIR);
  }
  process.chdir(LIB_WEBRTC_DIR);

  // Download libwebrtc
  var child = exec(GCLIENT+" config http://webrtc.googlecode.com/svn/trunk");
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

  process.chdir(LIB_WEBRTC_DIR);

  var child = spawn(GCLIENT, ["sync"]);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal)
  {
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

  process.chdir(LIB_WEBRTC_DIR);

  var child = spawn(GCLIENT, ["runhooks"]);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal)
  {
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

  process.chdir(LIB_WEBRTC_DIR);

  var args = ["-C", "trunk/out/Release"];
  if('linux' == os.platform())
  {
    args.push("peerconnection_client");
  }

  var child = spawn("ninja", args);
  child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
  child.on('exit', function(code, signal)
  {
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

depot_tools(function()
{
  console.log('wrtc build complete');
});
