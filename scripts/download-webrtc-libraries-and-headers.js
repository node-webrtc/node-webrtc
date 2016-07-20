#!/usr/bin/env node

var download = require('download');
var execSync = require('child_process').execSync;
var fs = require('fs');
var gunzip = require('gunzip-maybe');
var path = require('path');
var tar = require('tar-fs');
var url = require('url');

/**
 * @typedef {object} Options
 * @property {string} [arch] - the architecture to download or build for
 * @property {BinaryOptions} [binary] - node-pre-gyp-style configuration
 * @property {string} [branch] - the WebRTC branch head
 * @property {boolean} [buildWebRTC] - whether or not to build from source if binaries are unavailable
 * @property {string} [buildWebRTCDependency] - the build-webrtc package to use
 * @property {string} [commit] - the WebRTC commit
 * @property {string} [platform] - the platform to download or build for
 * @property {boolean} [skipDownload] - skipDownload implies buildWebRTC
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
  arch: process.arch,
  binary: {
    host: 'https://webrtc-libraries-and-headers.s3.amazonaws.com',
    module_name: 'webrtc',
    module_path: 'third_party/webrtc',
    remote_path: 'v1/build',
    package_name: '{module_name}-{branch}+{commit}.{platform}.{arch}.tar.gz'
  },
  buildWebRTC: false,
  buildWebRTCDependency: 'build-webrtc@0.1',
  branch: '50',
  commit: '49f7bd3',
  platform: process.platform,
  skipDownload: false
};

var options = {
  arch: process.env.TARGET_ARCH || defaults.arch,
  binary: defaults.binary,
  buildWebRTC: process.env.BUILD_WEBRTC ? true : defaults.buildWebRTC,
  buildWebRTCDependency: process.env.BUILD_WEBRTC_DEPENDENCY || defaults.buildWebRTCDependency,
  branch: process.env.WEBRTC_BRANCH || defaults.branch,
  commit: process.env.WEBRTC_COMMIT || defaults.commit,
  platform: process.env.TARGET_PLATFORM || defaults.platform,
  skipDownload: process.env.SKIP_DOWNLOAD ? true : defaults.skipDownload
};

/**
 * Perform variable substitution on a string.
 * @param {string} str
 * @param {Options} options
 * @returns {stirng}
 */
function printf(str, options) {
  return str
    .replace(/{arch}/g, options.arch)
    .replace(/{branch}/g, options.branch)
    .replace(/{commit}/g, options.commit)
    .replace(/{module_name}/g, options.binary.module_name)
    .replace(/{platform}/g, options.platform);
}

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
  var packageName = printf(options.binary.package_name, options);
  var remotePath = printf(options.binary.remote_path, options);
  return url.resolve(
    options.binary.host,
    remotePath + '/' + encodeURIComponent(packageName));
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
 * Attempt to download WebRTC libraries and headers. If they're unavailable,
 * fallback to building from source.
 * @param {Options} options
 * @returns {Promise}
 */
function downloadOrBuild(options) {
  var modulePath = computeModulePath(options.binary);
  if (exists(path.join(modulePath, 'include')) && exists(path.join(modulePath, 'lib'))) {
    return;
  } else if (options.skipDownload) {
    return build(options);
  }
  var url = computeUrl(options);
  console.log(
    'Attempting to download WebRTC libraries and headers for platform "%s" ' +
    'and architecture "%s" from\n', options.platform, options.arch);
  console.log('  %s\n', url);
  return new Promise(function(resolve, reject) {
    download(url)
      .once('error', reject)
      .pipe(gunzip())
      .once('error', reject)
      .pipe(tar.extract(modulePath))
      .once('error', reject)
      .once('finish', resolve);
  }).then(function() {
    console.log('Complete!');
  }, function(error) {
    if (error.statusCode === 404) {
      if (options.buildWebRTC) {
        console.log('Binaries unavailable! Falling back to building from source.');
        build(options);
        return;
      }
      console.error('Binaries unavailable! Try building from source by setting BUILD_WEBRTC=1.');
    } else {
      console.error(error);
    }
    process.exit(1);
  });
}

/**
 * Attempt to build WebRTC libraries from source.
 * @param {Options} options
 * @returns {undefined}
 */
function build(options) {
  var env = Object.assign({
    OUT: computeModulePath(options.binary),
    WEBRTC_REF: options.commit
  }, process.env);
  execSync('npm install ' + options.buildWebRTCDependency, {
    env: env,
    stdio: 'inherit'
  });
}

// Good luck!
downloadOrBuild(options);
