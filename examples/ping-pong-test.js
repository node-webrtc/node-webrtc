/* eslint no-console:0 */
'use strict';

var webrtc = require('..');

var RTCPeerConnection = webrtc.RTCPeerConnection;
var RTCSessionDescription = webrtc.RTCSessionDescription;

var pc1 = new RTCPeerConnection();
var pc2 = new RTCPeerConnection();

pc1.onicecandidate = function(candidate) {
  if (!candidate.candidate) return;
  pc2.addIceCandidate(candidate.candidate);
};

pc2.onicecandidate = function(candidate) {
  if (!candidate.candidate) return;
  pc1.addIceCandidate(candidate.candidate);
};

function handleError(error) {
  throw error;
}

var checks = 0;
var expected = 10;

function createDataChannels() {
  var dc1 = pc1.createDataChannel('test');
  dc1.onopen = function() {
    console.log('pc1: data channel open');
    dc1.onmessage = function(event) {
      var data = event.data;
      console.log('dc1: received "' + data + '"');
      console.log('dc1: sending "pong"');
      dc1.send('pong');
    };
  };

  var dc2;
  pc2.ondatachannel = function(event) {
    dc2 = event.channel;
    dc2.onopen = function() {
      console.log('pc2: data channel open');
      dc2.onmessage = function(event) {
        var data = event.data;
        console.log('dc2: received "' + data + '"');
        if (++checks === expected) {
          done();
        } else {
          console.log('dc2: sending "ping"');
          dc2.send('ping');
        }
      };
      console.log('dc2: sending "ping"');
      dc2.send('ping');
    };
  };

  createOffer();
}

function createOffer() {
  console.log('pc1: create offer');
  pc1.createOffer(setLocalDescription1, handleError);
}

function setLocalDescription1(desc) {
  console.log('pc1: set local description');
  pc1.setLocalDescription(
    new RTCSessionDescription(desc),
    setRemoteDescription2.bind(null, desc),
    handleError
  );
}

function setRemoteDescription2(desc) {
  console.log('pc2: set remote description');
  pc2.setRemoteDescription(
    new RTCSessionDescription(desc),
    createAnswer,
    handleError
  );
}

function createAnswer() {
  console.log('pc2: create answer');
  pc2.createAnswer(
    setLocalDescription2,
    handleError
  );
}

function setLocalDescription2(desc) {
  console.log('pc2: set local description');
  pc2.setLocalDescription(
    new RTCSessionDescription(desc),
    setRemoteDescription1.bind(null, desc),
    handleError
  );
}

function setRemoteDescription1(desc) {
  console.log('pc1: set remote description');
  pc1.setRemoteDescription(
    new RTCSessionDescription(desc),
    wait,
    handleError
  );
}

function wait() {
  console.log('waiting');
}

function run() {
  createDataChannels();
}

function done() {
  console.log('cleanup');
  pc1.close();
  pc2.close();
  console.log('done');
}

run();
