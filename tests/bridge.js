var http = require('http');
var webrtc = require('../index');
var ws = require('ws');

var MAX_REQUEST_LENGHT = 1024;
var pc = null;
var offer = null;
var answer = null;
var remoteReceived = false;

var dataChannelSettings = {
  'unreliable': {
        ordered: false,
        maxRetransmits: 0
      },
  /*
  'reliable': {},
  '@control': {
        outOfOrderAllowed: true,
        maxRetransmitNum: 0
      }
  */
};

var pendingDataChannels = {};
var dataChannels = {}
var pendingCandidates = [];

var args = process.argv;
var port = args[2] || 9001;

var wss = new ws.Server({'port': port});
wss.on('connection', function(ws)
{
  console.info('ws connected');
  function doComplete()
  {
    console.info('complete');
  }

  function doHandleError(error)
  {
    throw error;
  }

  function doCreateAnswer()
  {
    remoteReceived = true;
    pendingCandidates.forEach(function(candidate)
    {
      pc.addIceCandidate(new webrtc.RTCIceCandidate(candidate.sdp));
    });
    pc.createAnswer(
      doSetLocalDesc,
      doHandleError
    );
  };

  function doSetLocalDesc(desc)
  {
    answer = desc;
    console.info(desc);
    pc.setLocalDescription(
      desc,
      doSendAnswer,
      doHandleError
    );
  };

  function doSendAnswer()
  {
    ws.send(JSON.stringify(answer));
    console.log('awaiting data channels');
  }

  function doHandleDataChannels()
  {
    var labels = Object.keys(dataChannelSettings);
    pc.ondatachannel = function(channel) {
      console.log('ondatachannel', channel.label, channel.readyState);
      var label = channel.label;
      pendingDataChannels[label] = channel;
      channel.binaryType = 'arraybuffer';
      channel.onopen = function() {
        console.info('onopen');
        dataChannels[label] = channel;
        delete pendingDataChannels[label];
        if(Object.keys(dataChannels).length === labels.length) {
          doComplete();
        }
      };
      channel.onmessage = function(message) {
        console.log('onmessage', message);
      };
      channel.onclose = function() {
        console.info('onclose');
      };
      channel.onerror = doHandleError;
    };
    doSetRemoteDesc();
  };

  function doSetRemoteDesc()
  {
    console.info(offer);
    pc.setRemoteDescription(
      offer,
      doCreateAnswer,
      doHandleError
    );
  }

  ws.on('message', function(data)
  {
    data = JSON.parse(data);
    if('offer' == data.type)
    {
      offer = new webrtc.RTCSessionDescription(data);
      answer = null;
      remoteReceived = false;

      pc = new webrtc.RTCPeerConnection(
        {
          iceServers: [{url:'stun:stun.l.google.com:19302'}]
        },
        {
          'optional': [{DtlsSrtpKeyAgreement: true}]
        }
      );
      pc.onsignalingstatechange = function(state)
      {
        console.info('signaling state change:', state);
      }
      pc.onicecandidate = function(candidate)
      {
        ws.send(JSON.stringify(
          {'type': 'ice',
           'sdp': {'candidate': candidate.candidate, 'sdpMid': candidate.sdpMid, 'sdpMLineIndex': candidate.sdpMLineIndex}
          })
        );
      }

      doHandleDataChannels();
    } else if('ice' == data.type)
    {
      if(remoteReceived)
      {
        pc.addIceCandidate(new webrtc.RTCIceCandidate(data.sdp));
      } else
      {
        pendingCandidates.push(data);
      }
    }
  });
});