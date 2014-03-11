#!/usr/bin/env node
(function() {
  'use strict';

  var os = require('os');
  var fs = require('fs');
  var spawn = require('child_process').spawn;
  var argparse = require('argparse');

  var ARCH = process.arch;
  var PLATFORM = process.platform;
  var V8 = /[0-9]+\.[0-9]+/.exec(process.versions.v8)[0];
  var DEBUG = false;
  var FORCE = false;
})();