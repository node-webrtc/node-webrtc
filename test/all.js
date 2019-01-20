'use strict';

const semver = require('semver');

require('./closing-data-channel');
require('./closing-peer-connection');
require('./connect');
require('./create-offer');
require('./get-configuration');
require('./i420helpers');
require('./iceservers');
require('./mediastream');
require('./pass-interface-to-method');
require('./rtcdatachannel');
require('./rtcrtpreceiver');
require('./rtcvideosink');
require('./send-arraybuffer');
require('./sessiondesc');

// TODO(mroberts): This, and other tests in this block, use syntax incompatible
// with Node 6. Rewriting these tests to avoid async/await is not worth it, nor
// is introducing Babel.
//
// Therefore, we skip them. Remove this if-statement once we drop support for
// Node 6.
if (semver(process.version).major > 6) {
  require('./rtcvideosource');
}

// TODO(mroberts): These two tests use simple-peer, which introduced syntax
// incompatible with Node 6 in
//
//   https://github.com/feross/simple-peer/pull/355 introduced
//
// Therefore, we skip them. Remove this if-statement once we drop support for
// Node 6.
if (semver(process.version).major > 6) {
  require('./multiconnect');
  require('./custom-settings');
}

// TODO(mroberts): async_hooks were introduced in Node 9. We use them to test
// that destructors fire at the appropriate time (and hence, no memory leaks
// occur). Once we drop support for Node < 9, remove this.
if (semver(process.version).major >= 9 && typeof gc === 'function') {
  require('./destructor');
}
