'use strict';

/**
 * define window with webrtc functions for simple-peer
 */
if (typeof(window) === 'undefined') {
    var wrtc = require('..');
    global.window = {
        RTCPeerConnection: wrtc.RTCPeerConnection,
        RTCSessionDescription: wrtc.RTCSessionDescription,
        RTCIceCandidate: wrtc.RTCIceCandidate,
        // Firefox does not trigger "negotiationneeded"
        // this is a workaround to make simple-peer trigger the negotiation
        mozRTCPeerConnection: wrtc.RTCPeerConnection,
    };
}

var SimplePeer = require('simple-peer');


function bwtest(peer1, peer2) {
    var START_TIME = Date.now();
    var BUFFERED_DELAY_MS = 5;
    var NPACKETS = 5000;
    var PACKET_SIZE = 16 * 1024;
    var CONGEST_HIGH_THRESHOLD = 1 * 1024 * 1024;
    var CONGEST_LOW_THRESHOLD = 256 * 1024;
    var buffer = new ArrayBuffer(PACKET_SIZE);
    var n = 0;
    var congested = 0;
    var stats = {
        count: 0,
        bytes: 0,
        congestCount: 0,
        congestTime: 0
    };

    function info() {
        var now = Date.now();
        if (congested) {
            stats.congestTime += Date.now() - congested;
            congested = now;
        }
        var took = (now - START_TIME) / 1000;
        var bufferedAmount = peer1._channel && peer1._channel.bufferedAmount;
        return 'sent ' + n + ' received ' + stats.count + '. ' +
            'congestion #' + stats.congestCount + ' ' +
            (stats.congestTime / 1024).toFixed(3) + ' seconds. ' +
            (bufferedAmount ? 'bufferedAmount ' + bufferedAmount + '. ' : '') +
            'took ' + took.toFixed(3) + ' seconds. ' +
            'bandwidth ' + (stats.bytes / took / 1024).toFixed(0) + ' KB/s. ';
    }

    peer2.on('data', function(data) {
        stats.count += 1;
        stats.bytes += data.length;
        if (stats.count >= NPACKETS) {
            console.log('RECEIVE DONE!', info());
            peer1.destroy();
            peer2.destroy();
        }
    });

    function congestion() {
        var bufferedAmount = peer1._channel && peer1._channel.bufferedAmount || 0;
        if ((bufferedAmount > CONGEST_HIGH_THRESHOLD) ||
            (congested && bufferedAmount > CONGEST_LOW_THRESHOLD)) {
            if (!congested) {
                congested = Date.now();
            }
            stats.congestCount += 1;
        } else {
            if (congested) {
                stats.congestTime += Date.now() - congested;
            }
            congested = 0;
        }
        return congested;
    }

    function send() {
        if (n >= NPACKETS) {
            console.log('SEND DONE!', info());
            return;
        }
        if (n % 100 === 0) {
            console.log('SENDING:', info());
        }
        if (congestion()) {
            setTimeout(send, BUFFERED_DELAY_MS);
            return;
        }
        peer1.send(buffer, function(err) {
            if (err) {
                console.error('ERROR!', info(), err.stack || err);
                return;
            }
            n += 1;
            if (global.setImmediate) {
                global.setImmediate(send);
            } else {
                setTimeout(send, 0);
            }
        });
    }

    send();
}

function init(callback) {
    var INITIATOR_CONFIG = {
        initiator: true,
        config: {
            iceServers: [{
                url: 'stun:23.21.150.121'
            }],

            /*
             * data channels are ordered by default - using unordered channel improves
             * performance but will likely require ordering in another app layer
             */
            ordered: false,

            /*
             * data channels are reliable by default - using either maxRetransmits
             * or maxPacketLifeTime will change to unreliable mode
             */
            // maxRetransmits: 5,
            // maxPacketLifeTime: 3000,
        }
    };
    var peer1 = new SimplePeer(INITIATOR_CONFIG);
    var peer2 = new SimplePeer();

    // when peer1 has signaling data, give it to peer2, and vice versa
    peer1.on('signal', peer2.signal.bind(peer2));
    peer2.on('signal', peer1.signal.bind(peer1));

    // wait for 'connect' event before using the data channel
    peer1.on('connect', callback.bind(null, peer1, peer2));
}

init(bwtest);
