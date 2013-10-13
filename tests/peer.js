(function() {

var webrtcSupported = true;

var RTCPeerConnection;
if(window.mozRTCPeerConnection)
  RTCPeerConnection = window.mozRTCPeerConnection;
else if(window.webkitRTCPeerConnection)
  RTCPeerConnection = window.webkitRTCPeerConnection;
else if(window.RTCPeerConnection)
  RTCPeerConnection = window.RTCPeerConnection
else
  webrtcSupported = false;

var RTCSessionDescription;
if(window.mozRTCSessionDescription)
  RTCSessionDescription = window.mozRTCSessionDescription;
else if(window.webkitRTCSessionDescription)
  RTCSessionDescription = window.webkitRTCSessionDescription;
else if(window.RTCSessionDescription)
  RTCSessionDescription = window.RTCSessionDescription
else
  webrtcSupported = false;

var RTCIceCandidate;
if(window.mozRTCIceCandidate)
  RTCIceCandidate = window.mozRTCIceCandidate;
else if(window.webkitRTCIceCandidate)
  RTCIceCandidate = window.webkitRTCIceCandidate;
else if(window.RTCIceCandidate)
  RTCIceCandidate = window.RTCIceCandidate;
else
  webrtcSupported = false;

var dataChannelSettings = {
  'unreliable': {
        reliable: false
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

function doHandleError(error)
{
  throw error;
}

function doComplete()
{
  console.log('complete');
}

function doWaitforDataChannels()
{
  console.log('awaiting data channels')
}

var ws = null;
var pc = new RTCPeerConnection(
  {
    iceServers: [{url:'stun:stun.l.google.com:19302'}]
  },
  {
    'optional': [{ 'RtpDataChannels': true }]
  }
);
pc.onsignalingstatechange = function(event)
{
  console.info('signaling state change', event);
}
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
}

doCreateDataChannels(doComplete);

function doCreateDataChannels(cb)
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
        cb.apply(undefined, []);
      }
    };
    channel.onclose = function(event) {
      console.info('onclose');
    }
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
  ws = new WebSocket("ws://localhost:9001");
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
  }
  ws.onmessage = function(event)
  {
    data = JSON.parse(event.data);
    if('answer' == data.type)
    {
      doSetRemoteDesc(data);
    } else if('ice' == data.type)
    {
      pc.addIceCandidate(new RTCIceCandidate(data.sdp));
    }
  }
}

function doSetRemoteDesc(desc)
{
  pc.setRemoteDescription(
    new RTCSessionDescription(desc),
    doWaitforDataChannels,
    doHandleError
  );
}

})();