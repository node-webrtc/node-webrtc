#!/usr/bin/sh


var PROJECT_DIR = process.cwd();

var SRC_DIR = PROJECT_DIR+'/src';
var NODE_MODULES = PROJECT_DIR+'/node_modules';


function read_stream(stream)
{
  stream.read();
  process.stdout.write('.');
}


function rm_rf(callback)
{
  console.info("rm_rf");
  process.stdout.write('.');

  // Download depot tools
  var child = exec("rm -rf build docs third_party");
  child.on('exit', function(code, signal)
  {
    process.stdout.write('\n');
    if(0 === code && !signal)
    {
      console.log('rm_rf complete');
      nodegyp_clean(callback);
    }
    else
    {
      console.error('rm_rf failed:', code, signal);
    }
  });
}

function nodegyp_clean(callback)
{
  console.info("nodegyp_clean");
  process.stdout.write('.');

  if(fs.existsSync(NODE_MODULES))
  {
    process.chdir(SRC_DIR);

    var child = spawn("node-gyp", ["clean"]);
    child.stdout.on('readable', read_stream.bind(undefined, child.stdout));
    child.on('exit', function(code, signal)
    {
      process.stdout.write('\n');
      if(0 === code && !signal)
      {
        console.log('nodegyp_clean complete');
        nodegyp_clean();
      }
      else
      {
        console.error('nodegyp_clean failed:', code, signal);
      }
    });
  }
  else
    callback();
};

function rm_rf_nodemodules(callback)
{
  console.info("rm_rf_nodemodules");
  process.stdout.write('.');

  process.chdir(PROJECT_DIR);

  // Download depot tools
  var child = exec("rm -rf node_modules");
  child.on('exit', function(code, signal)
  {
    process.stdout.write('\n');
    if(0 === code && !signal)
    {
      console.log('rm_rf_nodemodules complete');
      callback(callback);
    }
    else
    {
      console.error('rm_rf_nodemodules failed:', code, signal);
    }
  });
}


rm_rf(function()
{
  console.log('wrtc uninstall complete');
});