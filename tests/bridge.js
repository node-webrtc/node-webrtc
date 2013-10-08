var http = require('http');
var webrtc = require('../index');

var MAX_REQUEST_LENGHT = 1024;
var pc = null;

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
      console.info('signaling state:', pc.signalingState);
    }

    var offer = new webrtc.RTCSessionDescription(JSON.parse(data));
    pc = new webrtc.RTCPeerConnection();
    pc.setRemoteDescription(
      offer,
      doCreateAnswer,
      doHandleError
    );
    pc.onsignalingstatechange = function(state)
    {
      console.info('signaling state change:', state);
    }
    pc.onicecandidate = function(candidate)
    {
      console.info(candidate);
    }
  });
}

var http_server = http.createServer(request_handler);
http_server.listen(9001);