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
    portRange: {}
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

function testConfiguration(t, actualConfiguration, expectedConfiguration) {
  t.deepEqual(actualConfiguration, expectedConfiguration);
}
