var http = require('http');
var webrtc = require('../index');

var MAX_REQUEST_LENGHT = 1024;
var pc = null;
pc = new webrtc.RTCPeerConnection();

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

function doHandleError(error)
{
  throw error;
}

function request_handler(req, res)
{
  if(req.method !== 'POST')
  {
    return res.end();
  }

  var data = '';

  req.on('data', function(chunk)
  {
    if(data.length > MAX_REQUEST_LENGHT)
    {
      res.end();
      return;
    }

    data += chunk;
  });

  req.on('end', function(chunk)
  {
    if(chunk)
    {
      data += chunk;
    }

    function doComplete()
    {
      console.info('complete');
    }

    function doCreateAnswer()
    {
      pc.createAnswer(
        doSetLocalDesc,
        doHandleError
      );
    };

    var answer = null;
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
      res.statusCode = 200;
      res.setHeader('Access-Control-Allow-Origin', '*');
      res.setHeader('Content-Type', 'application/json');
      res.end(JSON.stringify(answer));
      console.log('awaiting data channels');
    }

    function doHandleDataChannels()
    {
      var labels = Object.keys(dataChannelSettings);
      pc.ondatachannel = function(event) {
        console.log('ondatachannel');
        var channel = event.channel;
        var label = channel.label;
        pendingDataChannels[label] = channel;
        channel.binaryType = 'arraybuffer';
        channel.onopen = function() {
          console.error('onopen');
          dataChannels[label] = channel;
          delete pendingDataChannels[label];
          if(Object.keys(dataChannels).length === labels.length) {
            doComplete();
          }
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

    var offer = new webrtc.RTCSessionDescription(JSON.parse(data));
    doHandleDataChannels();

    pc.onsignalingstatechange = function(state)
    {
      console.info('signaling state change:', state);
      console.info('ready state:', pc.readyState);
    }
    pc.onicecandidate = function(candidate)
    {
      console.info('candidate:', candidate);
    }
  });
}

var http_server = http.createServer(request_handler);
http_server.listen(9001);