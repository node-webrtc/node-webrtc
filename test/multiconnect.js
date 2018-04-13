/* eslint no-console:0, no-process-env:0 */
'use strict';

var tape = require('tape');
var SimplePeer = require('simple-peer');
var wrtc = require('..');

var log = process.env.LOG ? console.log : function() {};

tape('connect once', function(t) {
  t.plan(1);
  log('###########################\n');
  connect(function(err) {
    t.error(err, 'connect callback');
  });
});

tape('connect loop', function(t) {
  t.plan(1);
  log('###########################\n');
  connectLoop(10, function(err) {
    t.error(err, 'connect callback');
  });
});

tape('connect concurrent', function(t) {
  var n = 10;
  t.plan(n);
  log('###########################\n');
  for (var i = 0; i < n; i += 1) {
    connect(callback);
  }

  function callback(err) {
    t.error(err, 'connect callback');
  }
});

tape('connect loop concurrent', function(t) {
  var n = 10;
  t.plan(n);
  log('###########################\n');
  for (var i = 0; i < n; i += 1) {
    connectLoop(10, callback);
  }

  function callback(err) {
    t.error(err, 'connect callback');
  }
});

var connIdGen = 1;

function connect(callback) {
  var connId = connIdGen;
  var connName = 'CONNECTION-' + connId;
  connIdGen += 1;
  log(connName, 'starting');

  // setup two peers with simple-peer
  var peer1 = new SimplePeer({
    wrtc: wrtc
  });
  var peer2 = new SimplePeer({
    wrtc: wrtc,
    initiator: true
  });

  function cleanup() {
    if (peer1) {
      peer1.destroy();
      peer1 = null;
    }
    if (peer2) {
      peer2.destroy();
      peer2 = null;
    }
  }

  // when peer1 has signaling data, give it to peer2, and vice versa
  peer1.on('signal', function(data) {
    log(connName, 'signal peer1 -> peer2:');
    log(' ', data);
    peer2.signal(data);
  });
  peer2.on('signal', function(data) {
    log(connName, 'signal peer2 -> peer1:');
    log(' ', data);
    peer1.signal(data);
  });

  peer1.on('error', function(err) {
    log(connName, 'peer1 error', err);
    cleanup();
    callback(err);
  });
  peer2.on('error', function(err) {
    log(connName, 'peer2 error', err);
    cleanup();
    callback(err);
  });

  // wait for 'connect' event
  peer1.on('connect', function() {
    log(connName, 'sending message');
    peer1.send('peers are for kids');
  });
  peer2.on('data', function() {
    log(connName, 'completed');
    cleanup();
    callback();
  });
}

function connectLoop(count, callback) {
  if (count <= 0) {
    log('connect loop completed');
    callback();
  } else {
    log('connect loop remain', count);
    connect(function(err) {
      if (err) {
        callback(err);
      } else {
        connectLoop(count - 1, callback);
      }
    });
  }
}
