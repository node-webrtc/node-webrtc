'use strict';

var tapSpec = require('tap-spec');

function makeConf(config) {
  var files = process.env.FILE
    ? [process.env.FILE]
    : [];

  var preprocessors = files.reduce((preprocessors, file) => {
    preprocessors[file] = 'browserify';
    return preprocessors;
  }, {});

  var browsers = {
    chrome: ['ChromeWebRTC'],
    firefox: ['FirefoxWebRTC'],
  };

  if (process.env.BROWSER) {
    browsers = browsers[process.env.BROWSER];
    if (!browsers) {
      throw new Error('Unknown browser');
    }
  } else {
    browsers = ['ChromeWebRTC', 'FirefoxWebRTC'];
  }

  config.set({
    browsers: browsers,
    concurrency: 1,
    customLaunchers: {
      ChromeWebRTC: {
        base: 'Chrome',
        flags: [
          '--use-fake-device-for-media-stream',
          '--use-fake-ui-for-media-stream'
        ]
      },
      FirefoxWebRTC: {
        base: 'Firefox',
        prefs: {
          'media.gstreamer.enabled': false,
          'media.navigator.permission.disabled': true,
          'media.navigator.streams.fake': true
        }
      }
    },
    files: files,
    frameworks: ['browserify', 'tap'],
    preprocessors: preprocessors,
    reporters: ['tap-pretty'],
    singleRun: true,
    tapReporter: { prettify: tapSpec }
  });
};

module.exports = makeConf;
