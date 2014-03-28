'use strict';

var test = require('tape');

var RTCPeerConnection = require('..').RTCPeerConnection;


var peer;
var localDesc;


test('create a peer connection', function(t) {
  t.plan(1);
  peer = new RTCPeerConnection({ iceServers: [] });
  t.ok(peer instanceof RTCPeerConnection, 'created');
});

test('createOffer function implemented', function(t) {
  t.plan(1);
  t.equal(typeof peer.createOffer, 'function', 'implemented');
});

test('can call createOffer', function(t) {

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

test('setLocalDescription function implemented', function(t) {
  t.plan(1);
  t.equal(typeof peer.setLocalDescription, 'function', 'implemented');
});

test('can call setLocalDescription', function(t) {
  var fail = t.ifError.bind(t);

  function pass() {
    t.ok(peer.localDescription, 'local description set');
    t.ok(peer.localDescription.sdp, 'we have local sdp');
  }

  t.plan(2);
  peer.setLocalDescription(localDesc, pass, fail);
});

test('TODO: cleanup connection', function(t) {
  t.plan(1);
  peer.close();
  t.pass('connection closed');
});