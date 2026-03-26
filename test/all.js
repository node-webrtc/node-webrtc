"use strict";

// require('child_process').spawnSync('pause', {shell: true, stdio: 'inherit'});
//
require("./addicecandidate");
require("./closing-data-channel");
require("./closing-peer-connection");
require("./connect");
require("./create-offer");
require("./custom-settings"); // Seems a little slow...
require("./destructor");
require("./get-configuration");
require("./get-settings");
require("./i420helpers");
require("./iceservers");
require("./mediastream");
require("./multiconnect"); // Seems a little slow...
require("./pass-interface-to-method");
require("./rollback");
require("./rtcaudiosink");
require("./rtcaudiosource");
require("./rtcdatachannel");
require("./rtcdtlstransport");
require("./rtcrtpreceiver");
require("./rtcrtpsender");
require("./rtcvideosink");
require("./rtcvideosource");
require("./send-arraybuffer");
require("./sessiondesc");
