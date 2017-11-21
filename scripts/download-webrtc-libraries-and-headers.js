#!/usr/bin/env node

var download = require('download');
var fs = require('fs');
var gunzip = require('gunzip-maybe');
var path = require('path');
var tar = require('tar-fs');
var unzip = require('unzip-stream');

/**
 * @typedef {object} Options
 * @property {string} [arch] - the architecture to download or build for
 * @property {BinaryOptions} [binary] - node-pre-gyp-style configuration
 * @property {string} [platform] - the platform to download or build for
 */

/**
 * These options are modeled after node-pre-gyp.
 * @typedef {object} BinaryOptions
 * @property {string} [module_path] - the path to extract the libraries and headers to
 */

var defaults = {
  arch: process.arch,
  binary: {
    module_path: 'third_party/webrtc'
  },
  platform: process.platform
};

var options = {
  arch: process.env.TARGET_ARCH || defaults.arch,
  binary: defaults.binary,
  platform: process.env.TARGET_PLATFORM || defaults.platform
};

/**
 * Compute the module path to extract WebRTC libraries and headers to.
 * @param {BinaryOptions} binary
 * @returns {string}
 */
function computeModulePath(binary) {
  return path.join(__dirname, '..', binary.module_path);
}

/**
 * Compute the URL of the WebRTC libraries and headers download.
 * @param {Options} options
 * @returns {string}
 */
function computeUrl(options) {
  var platform = {
    darwin: 'mac',
    win32: 'win'
  }[options.platform] || options.platform;
  var extension = platform === 'win' ? 'zip' : 'tar.gz';
  return 'https://github.com/aisouard/libwebrtc/releases/download/v1.0.0/libwebrtc-1.0.0-'
    + platform + '-' + options.arch + '.' + extension;
}

/**
 * Check whether a path exists.
 * @param {string} path
 * @returns {boolean}
 */
function exists(path) {
  try {
    fs.statSync(path);
    return true;
  } catch (error) {
    return false;
  }
}

/**
 * Attempt to download WebRTC libraries and headers.
 * @param {Options} options
 * @returns {Promise}
 */
function main(options) {
  var modulePath = computeModulePath(options.binary);
  if (exists(path.join(modulePath, 'include')) && exists(path.join(modulePath, 'lib'))) {
    return;
  }
  var url = computeUrl(options);
  console.log(
    'Attempting to download WebRTC libraries and headers for platform "%s" ' +
    'and architecture "%s" from\n', options.platform, options.arch);
  console.log('  %s\n', url);
  var extract = url.endsWith('.zip')
    ? unzip.Extract({ path: modulePath })
    : tar.extract(modulePath);
  return new Promise(function(resolve, reject) {
    download(url)
      .once('error', reject)
      .pipe(gunzip())
      .once('error', reject)
      .pipe(extract)
      .once('error', reject)
      .once('finish', resolve);
  }).then(function() {
    console.log('Complete!');
  }, function(error) {
    if (error.statusCode === 404) {
      console.error('Binaries unavailable!');
    } else {
      console.error(error);
    }
    process.exit(1);
  });
}

// Good luck!
main(options);
