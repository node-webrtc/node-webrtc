var test = require('tape');
test('Bridge Example', function(t) {

var host = window.location.host.split(':')[0];
var bridge = window.location.toString().split('?')[1] || host + ':8080';

var dataChannelSettings = {
  'reliable': {
        ordered: false,
        maxRetransmits: 10
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
var dataChannels = {};
var pendingCandidates = [];

function doHandleError(error)
{
  throw error;
}

function doComplete()
{
  console.log('complete');
  var data = new Uint8Array([97, 99, 107, 0]);
  dataChannels['reliable'].send(data.buffer);
  dataChannels['reliable'].send("Hello bridge!");
  t.pass('it worked');
  t.end();
}

function doWaitforDataChannels()
{
  console.log('awaiting data channels');
}

var ws = null;
var pc = new RTCPeerConnection(
  {
    iceServers: [{url:'stun:stun.l.google.com:19302'}]
  },
  {
    'optional': []
  }
);
pc.onsignalingstatechange = function(event)
{
  console.info("signaling state change: ", event.target.signalingState);
};
pc.oniceconnectionstatechange = function(event)
{
  console.info("ice connection state change: ", event.target.iceConnectionState);
};
pc.onicegatheringstatechange = function(event)
{
  console.info("ice gathering state change: ", event.target.iceGatheringState);
};
pc.onicecandidate = function(event)
{
  var candidate = event.candidate;
  if(!candidate) return;
  if(WebSocket.OPEN == ws.readyState)
  {
    ws.send(JSON.stringify(
      {'type': 'ice',
       'sdp': {'candidate': candidate.candidate, 'sdpMid': candidate.sdpMid, 'sdpMLineIndex': candidate.sdpMLineIndex}
      })
    );
  } else
  {
    pendingCandidates.push(candidate);
  }
};

doCreateDataChannels();

function doCreateDataChannels()
{
  var labels = Object.keys(dataChannelSettings);
  labels.forEach(function(label) {
    var channelOptions = dataChannelSettings[label];
    var channel = pendingDataChannels[label] = pc.createDataChannel(label, channelOptions);
    channel.binaryType = 'arraybuffer';
    channel.onopen = function() {
      console.info('onopen');
      dataChannels[label] = channel;
      delete pendingDataChannels[label];
      if(Object.keys(dataChannels).length === labels.length) {
        doComplete();
      }
    };
    channel.onmessage = function(event) {
      var data = event.data;
      if('string' == typeof data) {
        console.log('onmessage:', data);
      } else {
        console.log('onmessage:', new Uint8Array(data));
      }
    };
    channel.onclose = function(event) {
      console.info('onclose');
    };
    channel.onerror = doHandleError;
  });
  doCreateOffer();
}

function doCreateOffer()
{
  pc.createOffer(
    doSetLocalDesc,
    doHandleError
  );
}

function doSetLocalDesc(desc)
{
  pc.setLocalDescription(
    new RTCSessionDescription(desc),
    doSendOffer.bind(undefined, desc),
    doHandleError
  );
}

function doSendOffer(offer)
{
  ws = new WebSocket("ws://" + bridge);
  ws.onopen = function()
  {
    pendingCandidates.forEach(function(candidate)
    {
      ws.send(JSON.stringify(
        {'type': 'ice',
         'sdp': {'candidate': candidate.candidate, 'sdpMid': candidate.sdpMid, 'sdpMLineIndex': candidate.sdpMLineIndex}
        })
      );
    });
    ws.send(JSON.stringify(
      {'type': offer.type, 'sdp': offer.sdp})
    );
  };
  ws.onmessage = function(event)
  {
    data = JSON.parse(event.data);
    if('answer' == data.type)
    {
      doSetRemoteDesc(data);
    } else if('ice' == data.type)
    {
      if(data.sdp.candidate) {
        var candidate = new RTCIceCandidate(data.sdp.candidate);
        pc.addIceCandidate(candidate, handleAddIceCandidateSuccess, handleAddIceCandidateError);
      }
    }
  };
}

function handleAddIceCandidateSuccess() {

}

function handleAddIceCandidateError() {

}

function doSetRemoteDesc(desc)
{
  pc.setRemoteDescription(
    new RTCSessionDescription(desc),
    doWaitforDataChannels,
    doHandleError
  );
}

});
