'use strict';

var tape = require('tape');

var RTCPeerConnection = require('..').RTCPeerConnection;

tape('receiving two ArrayBuffers works', function(t) {
  var pc1 = new RTCPeerConnection();
  var pc2 = new RTCPeerConnection();
  [[pc1, pc2], [pc2, pc1]].forEach(function(pcs) {
    var pc1 = pcs[0];
    var pc2 = pcs[1];
    pc1.onicecandidate = function(event) {
      if (event.candidate) {
        pc2.addIceCandidate(event.candidate);
      }
    };
  });
  var dc1 = pc1.createDataChannel('test');
  var dc2Promise = new Promise(function(resolve) {
    pc2.ondatachannel = function(event) {
      resolve(event.channel);
    };
  });
  return pc1.createOffer().then(function(offer) {
    return Promise.all([
      pc1.setLocalDescription(offer),
      pc2.setRemoteDescription(offer)
    ]);
  }).then(function() {
    return pc2.createAnswer();
  }).then(function(answer) {
    return Promise.all([
      pc2.setLocalDescription(answer),
      pc1.setRemoteDescription(answer)
    ]);
  }).then(function() {
    return dc2Promise;
  }).then(function(dc2) {
    var buf1Promise = new Promise(function(resolve) {
      dc2.onmessage = function(event) {
        resolve(event.data);
      };
    });
    var buf1 = Buffer.alloc(1024, 0);
    dc1.send(buf1);
    return buf1Promise.then(function(remoteBuf1) {
      remoteBuf1 = new Uint8Array(remoteBuf1);
      var buf2Promise = new Promise(function(resolve) {
        dc2.onmessage = function(event) {
          resolve(event.data);
        };
      });
      var buf2 = Buffer.alloc(1024, 1);
      dc1.send(buf2);
      return buf2Promise.then(function(remoteBuf2) {
        remoteBuf2 = new Uint8Array(remoteBuf2);
        t.ok(remoteBuf1.every(function(x) {
          return x === 0;
        }), 'every element of the first ArrayBuffer is zero');
        t.ok(remoteBuf2.every(function(x) {
          return x === 1;
        }), 'every element of the second ArrayBuffer is one');
        t.end();
      });
    });
  }).then(function() {
    pc1.close();
    pc2.close();
  }, function(error) {
    pc1.close();
    pc2.close();
    throw error;
  });
});
