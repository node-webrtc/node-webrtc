'use strict';

const semver = require('semver');

require('./create-offer');
require('./sessiondesc');
require('./connect');
require('./iceservers');
// require('./bwtest').tape();
require('./mediastream');
require('./multiconnect');
require('./custom-settings');
require('./closing-peer-connection');
require('./closing-data-channel');
require('./get-configuration');
require('./rtcrtpreceiver');
require('./send-arraybuffer');
require('./rtcdatachannel');

if (semver(process.version).major >= 9 && typeof gc === 'function') {
  require('./destructor');
}
