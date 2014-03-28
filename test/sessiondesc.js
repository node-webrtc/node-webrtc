'use strict';

var test = require('tape');

var wrtc = require('..');

var RTCPeerConnection     = wrtc.RTCPeerConnection;
var RTCSessionDescription = wrtc.RTCSessionDescription;

var peer;
var localDesc;


test('create a peer connection', function(t) {
  t.plan(1);
  peer = new RTCPeerConnection({ iceServers: [] });
  t.ok(peer instanceof RTCPeerConnection, 'created');
});

test('createOffer', function(t) {

  var fail = t.ifError.bind(t);

  function pass(desc) {
    // save the local description
    localDesc = desc;

    // run the checks
    t.ok(desc, 'createOffer succeeded');
    t.equal(desc.type, 'offer', 'type === offer');
    t.ok(desc.sdp, 'got sdp');
  }

  t.plan(3);
  peer.createOffer(pass, fail);
});

test('setLocalDescription with a created RTCSessionDescription', function(t) {
  var fail = t.ifError.bind(t);

  function pass() {
    t.ok(peer.localDescription, 'local description set');
    t.ok(peer.localDescription.sdp, 'we have local sdp');
  }

  t.plan(2);
  peer.setLocalDescription(
    new RTCSessionDescription({ sdp: localDesc.sdp, type: 'offer' }),
    pass,
    fail
  );
});

test('TODO: cleanup connection', function(t) {
  t.plan(1);
  peer.close();
  t.pass('connection closed');
});