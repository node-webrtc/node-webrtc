'use strict';
var test = require('tape');

// var detect = require('rtc-core/detect');
// var RTCPeerConnection = detect('RTCPeerConnection');

var wrtc = require('..');

var RTCIceCandidate   = wrtc.RTCIceCandidate;
var RTCPeerConnection = wrtc.RTCPeerConnection;

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

test('capture ice candidates for peer:0', function(t) {
  t.plan(1);
  captureCandidates(peers[0], candidates[0], function() {
    t.equal(peers[0].iceGatheringState, 'complete', 'have candidates for peer:0');
  });
});

test('setRemoteDescription for peer:1', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[1].setRemoteDescription(peers[0].localDescription, pass, fail);
});

test('provide peer:1 with the peer:0 gathered ice candidates', function(t) {
  if (!candidates[0].length) {
    return t.end();
  }

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
    t.equal(peers[1].iceGatheringState, 'complete', 'have candidates for peer:1');
  });
});

test('setRemoteDescription for peer:0', function(t) {
  var fail = t.ifError.bind(t);
  var pass = t.pass.bind(t, 'ok');

  t.plan(1);
  peers[0].setRemoteDescription(peers[1].localDescription, pass, fail);
});

test('provide peer:0 with the peer:1 gathered ice candidates', function(t) {
  if (!candidates[1].length) {
    return t.end();
  }

  t.plan(candidates[1].length);

  candidates[1].forEach(function(candidate) {
    peers[0].addIceCandidate(
      new RTCIceCandidate(candidate),
      t.pass.bind(t, 'added candidate'),
      t.ifError.bind(t)
    );
  });
});

test('peer:1 triggers data channel event', function(t) {
  t.plan(2);
  peers[1].ondatachannel = function(evt) {
    dcs[1] = evt.channel;
    t.ok(dcs[1], 'got data channel');
    t.equal(dcs[1].label, 'test', 'data channel has correct label');
  };
});

test('monitor the ice connection state of peer:0', function(t) {
  t.plan(1);

  function checkState() {
    if (peers[0].iceConnectionState === 'connected' ||
        peers[0].iceConnectionState === 'completed') {
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

test('data channel connectivity', function(t) {
  t.plan(5);
  dcs[1].onmessage = function(evt) {
    var data = evt.data && new Uint8Array(evt.data);

    t.ok(data && typeof data.length !== 'undefined', 'got valid data');
    t.equal(data.length, 2, 'two bytes sent');
    t.equal(data[0], 10, 'byte:0 matches expected');
    t.equal(data[1], 11, 'byte:1 matches expected');
    // t.equal(evt.data, 'hello', 'dc:1 received message correctly');
  };

  dcs[0].send(new Uint8Array([10, 11]));
  t.pass('successfully called send on dc:0');
});

test('getStats', function(t) {
  t.plan(2);

  function getStats(peer, callback) {
    peer.getStats(function(response) {
      var reports = response.result();
      callback(null, reports.map(function(report) {
        var obj = {
          timestamp: report.timestamp,
          type: report.type
        };
        var names = report.names();
        names.forEach(function(name) {
          obj[name] = report.stat(name);
        });
        return obj;
      }));
    }, function(error) {
      callback(error);
    });
  }

  function done(error, reports) {
    if (error) {
      return t.fail(error);
    }
    console.log(reports);
    t.pass('successfully called getStats');
  }

  peers.forEach(function(peer) {
    getStats(peer, done);
  });
});

test('close the connections', function(t) {
  t.plan(3);

  peers[0].close();
  peers[1].close();

  // make sure nothing crashes after connection is closed and _jinglePeerConnection is null
  for (var i = 0; i < 2; i++) {
    peers[i].createOffer();
    peers[i].createAnswer();
    peers[i].setLocalDescription({}, function() {}, function(){});
    peers[i].setRemoteDescription({}, function() {}, function(){});
    peers[i].addIceCandidate({}, function() {}, function(){});
    peers[i].createDataChannel('test');
    peers[i].getStats(function(){}, function(err){
        t.ok(err);
    });
    peers[i].close();
  }

  t.pass('closed connections');

  peers = [];
});
