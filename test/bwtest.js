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
    var SEND_DELAY_MS = 1;
    var NPACKETS = 10000;
    var PACKET_SIZE = 16 * 1024;
    var buffer = new ArrayBuffer(PACKET_SIZE);
    var n = 0;
    var stats = {
        count: 0,
        bytes: 0,
    };

    function info() {
        var took = (Date.now() - START_TIME) / 1000;
        var buffered = peer1._channel && peer1._channel.bufferedAmount;
        return 'sent ' + n + ' received ' + stats.count +
            (buffered ? ' buffered ' + buffered : '') +
            ' took ' + took.toFixed(3) + ' seconds' +
            ' bandwidth ' + (stats.bytes / took / 1024).toFixed(0) + ' KB/s';
    }

    peer2.on('data', function(data) {
        stats.count += 1;
        stats.bytes += data.length;
        if (stats.count >= NPACKETS) {
            console.log('RECEIVE DONE!', info());
        }
    });

    function send() {
        if (n >= NPACKETS) {
            console.log('SEND DONE!', info());
            return;
        }
        if (n % 100 === 0) {
            console.log('SENDING:', info());
        }
        if (peer1._channel && peer1._channel.bufferedAmount) {
            console.log('WAITING:', info());
            setTimeout(send, SEND_DELAY_MS);
            return;
        }
        peer1.send(buffer, function(err) {
            if (err) {
                console.error('ERROR!', info(), err.stack || err);
                return;
            }
            n += 1;
            setTimeout(send, SEND_DELAY_MS);
        });
    }

    send();
}

function init(callback) {
    var INITIATOR_CONFIG = {
        initiator: true
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
