/* eslint no-console:0 */
'use strict';

var tape = require('tape');
var SimplePeer = require('simple-peer');
var wrtc = require('..');

tape('custom ports connect once', function(t) {
  t.plan(1);
  connectClientServer({ min: 9000, max: 9010 }, function(err) {
    t.error(err, 'connectClientServer callback');
  });
});

tape('custom ports connect concurrently', function(t) {
  const n = 2;

  t.plan(n);
  const portRange = { min: 9000, max: 9010 };

  function callback(err) {
    t.error(err, 'connectClientServer callback');
  }

  for (let i = 0; i < n; i++) {
    connectClientServer(portRange, callback);
  }
});

function connectClientServer(portRange, callback) {
  const client = new SimplePeer({
    wrtc: wrtc,
    initiator: true
  });

  const server = new SimplePeer({
    wrtc: wrtc,
    initiator: false,
    config: {
      portRange: portRange
    }
  });

  client.on('signal', function(data) {
    server.signal(data);
  });
  server.on('signal', function(data) {
    if (data.candidate && !isValidCandidate(data.candidate.candidate, portRange || { min: 0, max: 65535 }, true)) {
      callback(`candidate must follow port range (${portRange}): ${data.candidate.candidate}`);
    }
    client.signal(data);
  });
  server.on('connect', function() {
    server.send('xyz');
  });
  client.on('data', function() {
    callback();
    server.destroy();
    client.destroy();
  });
  client.on('error', function(e) {
    callback(e);
    server.destroy();
    client.destroy();
  });
  server.on('error', function(e) {
    callback(e);
    client.destroy();
    server.destroy();
  });
}

function isValidCandidate(candidate, portRange) {
  const port = candidate.replace(/candidate:([^\s]+)\s([^\s]+)\s([^\s]+)\s([^\s]+)\s([^\s]+)\s([0-9]+)\styp.*/, '$6');

  const minPort = portRange.min;
  const maxPort = portRange.max;

  return minPort <= parseInt(port) && maxPort >= parseInt(port);
}
