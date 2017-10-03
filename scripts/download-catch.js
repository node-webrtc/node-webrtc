#!/usr/bin/env node

var download = require('download');
var execSync = require('child_process').execSync;
var fs = require('fs');
var gunzip = require('gunzip-maybe');
var mkdirp = require('mkdirp').sync;
var path = require('path');
var tar = require('tar-fs');
var url = require('url');

/**
 * @typedef {object} Options
 * @property {string} [url] - the URL of the Catch header to use
 */

/**
 * These options are modeled after node-pre-gyp.
 * @typedef {object} BinaryOptions
 * @property {string} [host] - the host to download libraries and headers from
 * @property {string} [module_name] - the name of the libraries and headers
 * @property {string} [module_path] - the path to extract the libraries and headers to
 * @property {string} [remote_path] - the path to the libraries and headers on the host
 * @property {string} [package_name] - the name of the package on the host
 */

var defaults = {
  url: 'https://github.com/philsquared/Catch/releases/download/v1.10.0/catch.hpp'
};

var options = {
  url: defaults.url
};

/**
 * Compute the path to the Catch directory.
 * @returns {string}
 */
function computeCatchDir() {
  return path.join(__dirname, '..', 'third_party', 'catch');
}

/**
 * Compute the path to the Catch header.
 * @returns {string}
 */
function computeCatchPath() {
  return path.join(computeCatchDir(), 'catch.hpp');
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
 * Attempt to download Catch.
 * @param {Options} options
 * @returns {Promise}
 */
function main(options) {
  var catchDir = computeCatchDir();
  mkdirp(catchDir);
  var catchPath = computeCatchPath();
  if (exists(catchPath)) {
    return;
  }
  var url = options.url;
  console.log('Attempting to download the Catch header from\n');
  console.log('  %s\n', url);
  return new Promise(function(resolve, reject) {
    download(url)
      .once('error', reject)
      .pipe(fs.createWriteStream(catchPath))
      .once('error', reject)
      .once('finish', resolve);
  }).then(function() {
    console.log('Complete!');
  }, function(error) {
    console.error(error);
    process.exit(1);
  });
}

// Good luck!
main(options);
