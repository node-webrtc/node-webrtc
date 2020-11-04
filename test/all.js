'use strict';

// const semver = require('semver');

require('./addicecandidate');
require('./closing-data-channel');
require('./closing-peer-connection');
require('./connect');
require('./create-offer');
require('./custom-settings');
require('./get-configuration');
require('./i420helpers');
require('./iceservers');
require('./mediastream');
require('./multiconnect');
require('./pass-interface-to-method');
require('./rollback');
require('./rtcaudiosink');
require('./rtcaudiosource');
require('./rtcdtlstransport');
require('./rtcdatachannel');
require('./rtcrtpreceiver');
require('./rtcrtpsender');
require('./rtcvideosink');
require('./rtcvideosource');
require('./send-arraybuffer');
require('./sessiondesc');

// TODO(mroberts): async_hooks were introduced in Node 9. We use them to test
// that destructors fire at the appropriate time (and hence, no memory leaks
// occur). Once we drop support for Node < 9, remove this.
// TODO: this need to be fixed
// if (semver(process.version).major >= 9 && typeof gc === 'function') {
//   require('./destructor');
// }
