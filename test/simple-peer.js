'use strict';


module.exports = requireSimplePeer();

/**
 * 
 * define window with webrtc functions before loading simple-peer
 *
 * NOTE: in new version of simple-peer it already supports
 * nodejs wrtc so this will be removed.
 *
 */
function requireSimplePeer() {
    if (typeof(window) === 'undefined') {
        var wrtc = require('..');
        global.window = {
            RTCPeerConnection: wrtc.RTCPeerConnection,
            RTCSessionDescription: wrtc.RTCSessionDescription,
            RTCIceCandidate: wrtc.RTCIceCandidate,
            // node-webrtc and Firefox do not trigger "negotiationneeded"
            // this is a workaround to make simple-peer trigger the negotiation
            mozRTCPeerConnection: wrtc.RTCPeerConnection,
        };
    }
    return require('simple-peer');
}
