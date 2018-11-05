'use strict';

var test = require('tape');

var RTCPeerConnection = require('..').RTCPeerConnection;

test('getConfiguration', function(t) {
  var defaultConfiguration = {
    iceServers: [],
    iceTransportPolicy: 'all',
    bundlePolicy: 'balanced',
    rtcpMuxPolicy: 'require',
    iceCandidatePoolSize: 0,
    portRange: {},
    sdpSemantics: 'plan-b'
  };

  t.test('before calling close, with defaults', function(t) {
    var pc = new RTCPeerConnection();
    var actualConfiguration = pc.getConfiguration();
    pc.close();
    testConfiguration(t, actualConfiguration, defaultConfiguration);
    t.end();
  });

  t.test('after calling close, with defaults', function(t) {
    var pc = new RTCPeerConnection();
    pc.close();
    var actualConfiguration = pc.getConfiguration();
    testConfiguration(t, actualConfiguration, defaultConfiguration);
    t.end();
  });

  [
    ['iceServers', [
      {
        urls: 'stun:stun1.example.net'
      },
      {
        urls: ['turns:turn.example.org', 'turn:turn.example.net'],
        username: 'user',
        credential: 'myPassword',
        credentialType: 'password'
      }
    ]],
    ['iceTransportPolicy', 'relay'],
    ['bundlePolicy', 'max-bundle'],
    ['rtcpMuxPolicy', 'negotiate'],
    ['iceCandidatePoolSize', 255],
    ['portRange', { min: 1, max: 2 }]
  ].forEach(function(pair) {
    t.test('after setting ' + pair[0], function(t) {
      var expectedConfiguration = Object.assign({}, defaultConfiguration);
      expectedConfiguration[pair[0]] = pair[1];
      var pc = new RTCPeerConnection(expectedConfiguration);
      pc.close();
      var actualConfiguration = pc.getConfiguration();
      testConfiguration(t, actualConfiguration, expectedConfiguration);
      t.end();
    });
  });
});

test('setConfiguration', function(t) {
  t.test('after closing', function(t) {
    var pc = new RTCPeerConnection();
    pc.close();
    t.throws(pc.setConfiguration.bind(pc, {}));
    t.end();
  });

  t.test('changing iceServers', function(t) {
    var pc = new RTCPeerConnection();
    var expectedConfiguration = Object.assign({}, pc.getConfiguration());
    expectedConfiguration.iceServers = [
      { urls: 'stun:stun1.example.net' }
    ];
    pc.setConfiguration(expectedConfiguration);
    var actualConfiguration = pc.getConfiguration();
    testConfiguration(t, actualConfiguration, expectedConfiguration);
    pc.close();
    t.end();
  });

  t.test('changing iceServers', function(t) {
    var pc = new RTCPeerConnection();
    var expectedConfiguration = Object.assign({}, pc.getConfiguration());
    expectedConfiguration.iceTransportPolicy = 'relay';
    pc.setConfiguration(expectedConfiguration);
    var actualConfiguration = pc.getConfiguration();
    testConfiguration(t, actualConfiguration, expectedConfiguration);
    pc.close();
    t.end();
  });

  // If the value of configuration.bundlePolicy differs from the connection's
  // bundle policy, throw an InvalidModificationError.
  t.test('changing bundlePolicy throws', function(t) {
    var pc = new RTCPeerConnection({ bundlePolicy: 'max-bundle' });
    t.throws(pc.setConfiguration.bind(pc, { bundlePolicy: 'max-compat' }));
    pc.close();
    t.end();
  });

  // If the value of configuration.rtcpMuxPolicy differs from the connection's
  // rtcpMux policy, throw an InvalidModificationError.
  t.test('changing rtcpMuxPolicy throws', function(t) {
    var pc = new RTCPeerConnection({ rtcpMuxPolicy: 'negotiate' });
    t.throws(pc.setConfiguration.bind(pc, { rtcpMuxPolicy: 'require' }));
    pc.close();
    t.end();
  });

  // If the value of configuration.iceCandidatePoolSize differs from the
  // connection's previously set iceCandidatePoolSize, and setLocalDescription
  // has already been called, throw an InvalidModificationError.
  t.test('changing iceCandidatePoolSize after setLocalDescription throws', function(t) {
    var pc = new RTCPeerConnection({ iceCandidatePoolSize: 1 });
    pc.createOffer().then(function(offer) {
      pc.setConfiguration({ iceCandidatePoolSize: 2 });
      return pc.setLocalDescription(offer);
    }).then(function() {
      t.throws(pc.setConfiguration.bind(pc, { iceCandidatePoolSize: 3 }));
      pc.close();
      t.end();
    });
  });
});

function testConfiguration(t, actualConfiguration, expectedConfiguration) {
  t.deepEqual(actualConfiguration, expectedConfiguration);
}
