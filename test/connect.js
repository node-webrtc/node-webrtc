var test = require('tape');
var RTCPeerConnection = require('../peerconnection');
var RTCDataChannel = require('../datachannel');
var peers = [];
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

test('create a datachannel on peer:0', function(t) {
  t.plan(2);
  t.ok(dcs[0] = peers[0].createDataChannel('test'));
  t.equal(dcs[0].label, 'test', 'created with correct label');
});

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

test('setRemoteDescription for peer:1', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[1].setRemoteDescription(peers[0].localDescription, pass, fail);
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

test('setRemoteDescription for peer:0', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[0].setRemoteDescription(peers[1].localDescription, pass, fail);
});

test('close the connections', function(t) {
  t.plan(1);
  peers[0].close();
  peers[1].close();
  t.pass('closed connections');

  peers = [];
});