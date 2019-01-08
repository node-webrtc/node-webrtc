'use strict';

const semver = require('semver');

require('./pass-interface-to-method');
require('./create-offer');
require('./sessiondesc');
require('./connect');
require('./iceservers');
// require('./bwtest').tape();
require('./mediastream');
// NOTE(mroberts): https://github.com/feross/simple-peer/pull/355 introduced
// syntax incompatible with Node 6.
if (semver(process.version).major > 6) {
  require('./multiconnect');
  require('./custom-settings');
}
require('./closing-peer-connection');
require('./closing-data-channel');
require('./get-configuration');
require('./rtcrtpreceiver');
require('./send-arraybuffer');
require('./rtcdatachannel');

if (semver(process.version).major >= 9) {
  require('./rtcvideosource');
  if (typeof gc === 'function') {
    require('./destructor');
  }
}
