/* eslint no-console:0, no-process-env:0 */
'use strict';

var test = require('tape');

var wrtc = require('..');

var RTCPeerConnection = wrtc.RTCPeerConnection;
var RTCSessionDescription = wrtc.RTCSessionDescription;

// NOTE(mroberts): Not sure why we need to do this. This may be a firewall issue.
var isDarwinOnCircleCi = process.platform === 'darwin'
  && process.env.CIRCLECI === 'true';

var skipReflexive = isDarwinOnCircleCi || !process.env.CHECK_REFLEXIVE;

var pc;

test('assign ICE server and get reflective candidates', function(t) {
  t.plan(1);

  pc = new RTCPeerConnection({
    iceServers: [
      {
        urls: ['stun:stun.l.google.com:19302']
      }
    ]
  });

  var gotReflective = false;

  function finish() {
    if (pc.signalingState === 'closed') {
      return;
    }
    pc.close();
    t.equal(gotReflective || skipReflexive, true, 'gotReflective === true');
  }

  pc.onicecandidate = function(candidate) {
    if (candidate.candidate) {
      console.log(candidate.candidate.candidate);
      if (candidate.candidate.candidate.indexOf('typ srflx') > -1) {
        gotReflective = true;
        finish();
      }
    }
  };

  pc.onicegatheringstatechange = function() {
    if (pc.iceGatheringState === 'complete') {
      finish();
    }
  };

  pc.createDataChannel('test');

  pc.createOffer().then(function(e) {
    pc.setLocalDescription(new RTCSessionDescription(e));
  });
});

test('dont assign ICE server and get no reflective candidates', function(t) {
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

  pc.createDataChannel('test');

  pc.createOffer().then(function(e) {
    pc.setLocalDescription(new RTCSessionDescription(e));
  });
});
