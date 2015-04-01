'use strict';

var wrtc = require('..');

/**
 * define window for simple-peer
 */
global.window = {
    RTCPeerConnection: wrtc.RTCPeerConnection,
    RTCSessionDescription: wrtc.RTCSessionDescription,
    RTCIceCandidate: wrtc.RTCIceCandidate,
    // Firefox does not trigger "negotiationneeded"
    // this is a workaround to make simple-peer trigger the negotiation
    mozRTCPeerConnection: wrtc.RTCPeerConnection,
};

var SimplePeer = require('simple-peer');


function bwtest(peer1, peer2) {
    var stats = {
        count: 0,
        bytes: 0,
    };
    var SEND_DELAY_MS = 1;
    var NPACKETS = 100000;
    var PACKET_SIZE = 16 * 1024;
    var buffer = new ArrayBuffer(PACKET_SIZE);
    var n = 0;

    peer2.on('data', function(data) {
        stats.count += 1;
        stats.bytes += data.length;
    });

    function send() {
        if (n >= NPACKETS) {
            console.log('DONE!', stats);
            return;
        }
        console.log('send', n, stats);
        peer1.send(buffer, function(err) {
            if (err) {
                console.error('ERROR!', stats);
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
