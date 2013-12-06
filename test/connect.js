var test = require('tape');
var RTCPeerConnection = require('../peerconnection');
var peers = [];
var dcs = [];
var peerpair = require('peerpair');

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

// test('can create a datachannel', function(t) {
//   t.plan(2);
//   t.ok(dcs[0] = peers[0].createDataChannel('test'));
//   t.equal(dcs[0].label, 'test');
// });

test('pair the connections', function(t) {
  t.plan(1);
  peers = peerpair(peers, {
    RTCSessionDescription: require('../sessiondescription')
  });
  t.pass('paired');
});

test('can connect the peers', function(t) {
  t.plan(2);

  peers.events.once('connected', function() {
    t.equal(peers[0].iceConnectionState, 'connected');
    t.equal(peers[1].iceConnectionState, 'connected');
  });

  peers.connect();
});

// test('release the connection references', function(t) {
//   peers = [];
// });

// test('close the connections', function(t) {
//   t.plan(2);
//   peers[0].close();
//   peers[1].close();
// });