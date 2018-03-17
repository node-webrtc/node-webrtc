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
