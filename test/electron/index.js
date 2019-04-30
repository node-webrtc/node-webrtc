/* eslint no-process-exit:0 */
'use strict';

const { RTCPeerConnection } = require('../..');

const pc = new RTCPeerConnection();

pc.close();

process.exit();
