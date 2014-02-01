var test = require('tape');
// var detect = require('rtc-core/detect');
// var RTCPeerConnection = detect('RTCPeerConnection');
var RTCPeerConnection = require('../lib/peerconnection');
var RTCIceCandidate = require('../lib/icecandidate');
var captureCandidates = require('./helpers/capture-candidates');
var peers = [];
var candidates = [ [], [] ];
var dcs = [];
var localDesc;

test('create the peer connections', function(t) {
  t.plan(2);
  peers = [
    new RTCPeerConnection({ iceServers: [] }),
    new RTCPeerConnection({ iceServers: [] })
  ];

  t.ok(peers[0] instanceof RTCPeerConnection, 'peer:0 created ok');
  t.ok(peers[1] instanceof RTCPeerConnection, 'peer:1 created ok');
});

test('peers are created and in the expected connection state', function(t) {
  t.plan(2);
  t.equal(peers[0].iceConnectionState, 'new');
  t.equal(peers[1].iceConnectionState, 'new');
});

// test('create a datachannel on peer:0', function(t) {
//   t.plan(2);
//   t.ok(dcs[0] = peers[0].createDataChannel('test'));
//   t.equal(dcs[0].label, 'test', 'created with correct label');
// });

test('createOffer for peer:0', function(t) {

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
  peers[0].createOffer(pass, fail);
});

test('setLocalDescription for peer:0', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[0].setLocalDescription(localDesc, pass, fail);
});

test('capture ice candidates for peer:0', function(t) {
  t.plan(1);
  captureCandidates(peers[0], candidates[0], function() {
    t.ok(candidates[0].length > 0, 'have candidates for peer:0');
  });
});

test('setRemoteDescription for peer:1', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[1].setRemoteDescription(peers[0].localDescription, pass, fail);
});

test('provide peer:1 with the peer:0 gathered ice candidates', function(t) {
  t.plan(candidates[0].length);

  candidates[0].forEach(function(candidate) {
    peers[1].addIceCandidate(
      new RTCIceCandidate(candidate),
      t.pass.bind(t, 'added candidate'),
      t.ifError.bind(t)
    );
  });
});

test('createAnswer for peer:1', function(t) {
  var fail = t.ifError.bind(t);

  function pass(desc) {
    // save the local description
    localDesc = desc;

    // run the checks
    t.ok(desc, 'createOffer succeeded');
    t.equal(desc.type, 'answer', 'type === answer');
    t.ok(desc.sdp, 'got sdp');
  }

  t.plan(3);
  peers[1].createAnswer(pass, fail);
});

test('setLocalDescription for peer:1', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[1].setLocalDescription(localDesc, pass, fail);
});

test('capture ice candidates for peer:1', function(t) {
  t.plan(1);
  captureCandidates(peers[1], candidates[1], function() {
    t.ok(candidates[1].length > 0, 'have candidates for peer:1');
  });
});

test('setRemoteDescription for peer:0', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[0].setRemoteDescription(peers[1].localDescription, pass, fail);
});

test('provide peer:0 with the peer:1 gathered ice candidates', function(t) {
  t.plan(candidates[1].length);

  candidates[1].forEach(function(candidate) {
    peers[0].addIceCandidate(
      new RTCIceCandidate(candidate),
      t.pass.bind(t, 'added candidate'),
      t.ifError.bind(t)
    );
  });
});

test('monitor the ice connection state of peer:0', function(t) {
  t.plan(1);

  function checkState() {
    if (peers[0].iceConnectionState === 'connected') {
      t.pass('peer:0 in connected state');
      peers[0].oniceconnectionstatechange = null;
    }
  }

  peers[0].oniceconnectionstatechange = checkState;
  checkState();
});

test('monitor the ice connection state of peer:1', function(t) {
  t.plan(1);

  function checkState() {
    if (peers[1].iceConnectionState === 'connected') {
      t.pass('peer:1 in connected state');
      peers[1].oniceconnectionstatechange = null;
    }
  }

  peers[1].oniceconnectionstatechange = checkState;
  checkState();
});

test('close the connections', function(t) {
  t.plan(1);
  peers[0].close();
  peers[1].close();
  t.pass('closed connections');

  peers = [];
});