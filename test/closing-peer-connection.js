'use strict';

var test = require('tape');
var wrtc = require('..');

var RTCPeerConnection = wrtc.RTCPeerConnection;

function onOpen(channel) {
  return new Promise(function(resolve) {
    channel.onopen = resolve;
  });
}

test('make sure channel is available after after connection is closed on the other side', function(t) {
  t.plan(2);

  var peer1 = new RTCPeerConnection();
  var peer2 = new RTCPeerConnection();

  [[peer1, peer2], [peer2, peer1]].forEach(function(peers) {
    var peer1 = peers[0];
    var peer2 = peers[1];
    peer1.onicecandidate = function(event) {
      if (event.candidate) {
        peer2.addIceCandidate(event.candidate);
      }
    };
  });

  var channel1 = peer1.createDataChannel('data', { negotiated: true, id: 0 });
  var channel2 = peer2.createDataChannel('data2', { negotiated: true, id: 0 });

  peer1.createOffer().then(function(offer) {
    return peer1.setLocalDescription(offer);
  }).then(function() {
    peer2.setRemoteDescription(peer1.localDescription);
    return peer2.createAnswer();
  }).then(function(answer) {
    return peer2.setLocalDescription(answer);
  }).then(function() {
    peer1.setRemoteDescription(peer2.localDescription);
    return Promise.all([
      onOpen(channel1),
      onOpen(channel2)
    ]);
  }).then(function() {
    channel2.close();
    t.equal(channel2.readyState, 'closed', 'can still check ready state after closing');
    peer2.close();
    setTimeout(function() {
      channel1.send('Hello');
      channel1.close();
      peer1.close();
      t.equal(channel1.readyState, 'closed', 'channel on the other side is also closed, but we did not crash');
    }, 100);
  });
});
