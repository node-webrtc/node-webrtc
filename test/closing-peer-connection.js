'use strict';

var test = require('tape');
var wrtc = require('..');

var RTCPeerConnection = wrtc.RTCPeerConnection;

test('make sure channel is available after after connection is closed on the other side', function(t) {
  t.plan(3);

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
  var channel2 = peer2.createDataChannel('data2', { negotiated: true, id: 0 });
  var waitingFor = 2;

  function ready() {
    --waitingFor;
    if (!waitingFor) {
      t.equal(channel2.readyState, 'open');
      channel2.close();
      t.ok(channel2.readyState === 'closing' || channel2.readyState === 'closed', 'can still check ready state after closing');
      peer2.close();
      setTimeout(function() {
        if (channel1.readyState === 'open') {
          channel1.send('Hello');
        }
        channel1.close();
        peer1.close();
        t.equal(channel1.readyState, 'closed', 'channel on the other side is also closed, but we did not crash');
      }, 100);
    }
  }

  channel1.onopen = ready;
  channel2.onopen = ready;

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
