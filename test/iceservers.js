'use strict';
var test = require('tape');

var wrtc = require('..');

var RTCPeerConnection       = wrtc.RTCPeerConnection;
var RTCSessionDescription   = wrtc.RTCSessionDescription;

var pc;
var gotReflective;

test('assign ICE server and get reflective candidates', function (t) {
    t.plan(1);

    pc = new RTCPeerConnection({
        iceServers: [
            {
                urls: ['stun:stun.l.google.com:19302']
            }
        ]
    });

    var gotReflective = false;

    pc.onicecandidate = function(candidate) {
        if (candidate.candidate) {
            console.log(candidate.candidate.candidate);
            if (candidate.candidate.candidate.indexOf('typ srflx') > -1) {
                gotReflective = true;
            }
        }
    };

    pc.onicegatheringstatechange = function() {
        if (pc.iceGatheringState === 'complete') {
            pc.close();

            t.equal(gotReflective, true, 'gotReflective === true');
        }
    };

    var dc = pc.createDataChannel('test');

    pc.createOffer().then(function(e) {
        pc.setLocalDescription(new RTCSessionDescription(e));
    });
});

test('dont assign ICE server and get no reflective candidates', function (t) {
    t.plan(1);

    pc = new RTCPeerConnection({
        iceServers: []
    });

    var gotReflective = false;

    pc.onicecandidate = function(candidate) {
        if (candidate.candidate) {
            console.log(candidate.candidate.candidate);
            if (candidate.candidate.candidate.indexOf('typ srflx') > -1) {
                gotReflective = true;
            }
        }
    };

    pc.onicegatheringstatechange = function() {
        if (pc.iceGatheringState === 'complete') {
            pc.close();

            t.equal(gotReflective, false, 'gotReflective === false');
        }
    };

    var dc = pc.createDataChannel('test');

    pc.createOffer().then(function(e) {
        pc.setLocalDescription(new RTCSessionDescription(e));
    });
});