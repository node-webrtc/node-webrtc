var test = require('tape');
var webrtc = require('..');
var conn;

test('create a connection', function(t) {
  t.plan(1);
  t.ok(conn = new webrtc.RTCPeerConnection());
});

test('close the connection', function(t) {
  t.plan(1);
  conn.close();
  t.pass('connection closed');
});