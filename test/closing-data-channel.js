'use strict';

var test = require('tape');
var RTCPeerConnection = require('..').RTCPeerConnection;

test('make sure closing an RTCDataChannel after an RTCPeerConnection has been garbage collected doesn\'t segfault', function(t) {
  var dc = (function() {
    var pc = new RTCPeerConnection();
    var dc = pc.createDataChannel('foo');
    pc.close();
    return dc;
  })();

  dc.close();
  t.end();
});

test('ensure that RTCDataChannel open is not called during closing state', function(t) {
  t.plan(5);

  var peer1 = new RTCPeerConnection({ iceServers: [] });
  var peer2 = new RTCPeerConnection({ iceServers: [] });

  [[peer1, peer2], [peer2, peer1]].forEach(function(peers) {
    peers[0].onicecandidate = function(event) {
      if (event.candidate) {
        peers[1].addIceCandidate(event.candidate);
      }
    };
  });

  var channel1 = peer1.createDataChannel('data', { negotiated: true, id: 0 });
  t.equal(channel1.readyState, 'connecting', 'Expected initial readyState to be "connecting"');

  var closeCount = 0;
  var waitingFor = 1;

  function ready() {
    t.equal(channel1.readyState, 'open', 'Expected readyState to be "open" in open');
    --waitingFor;
    if (!waitingFor) {
      peer1.close();
      peer2.close();
    }
  }

  function close() {
    ++closeCount;
    t.equal(closeCount, 1, 'Expected close count to be 1');
    t.equal(waitingFor, 0, 'Expected waiting for to be 0');
    t.equal(channel1.readyState, 'closed', 'successfully closed');
  }

  channel1.onopen = ready;
  channel1.onclose = close;

  peer1.createOffer()
    .then(function(offer) {
      return peer1.setLocalDescription(offer);
    })
    .then(function() {
      peer2.setRemoteDescription(peer1.localDescription);
      return peer2.createAnswer();
    })
    .then(function(answer) {
      return peer2.setLocalDescription(answer);
    })
    .then(function() {
      peer1.setRemoteDescription(peer2.localDescription);
    });
});
