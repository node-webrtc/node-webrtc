var http = require('http');
var path = require('path');

var webrtc = require('../../index');
var express = require('express');
var socketio = require('socket.io');

var router = express();
var server = http.createServer(router);
var io = socketio.listen(server);

router.use(express.static(__dirname + '/.'));

var configuration = { "iceServers": [{ "url": "stun:stun.example.org" }] };

io.set('transports',['xhr-polling']);

io.on('connection', function (socket) {
    socket.emit('connected');
    
    var pc = null;
    //var isHost = false;
    
    function logMessage(message) {
        console.log(message);
        message = 'server: ' + message;
        socket.emit('logmessage',message);
    }
    
    logMessage('client connected');
    
    function logError(error) {
        logMessage("Error: " + error);
    }
    
    function start() {
        logMessage('rtc peer connection object initializing');
        pc = new webrtc.RTCPeerConnection(configuration);
    
        pc.onicecandidate = function (evt) {
            if (evt.candidate) {
                socket.emit('message', JSON.stringify({ "candidate": evt.candidate }));
            }
        };
    
        pc.onnegotiationneeded = function () {
            pc.createOffer(localDescCreated, logError);
        };
    
        pc.onaddstream = function (evt) {
            logMessage('rtc remote stream added successfully');
        };
        
        /*if (isHost) {
            console.log('local stream added to rtc connection for broadcast');
            pc.addStream(media);
        }*/
    }
        
    function localDescCreated(desc) {
        pc.setLocalDescription(desc, function () {
            socket.emit('message', JSON.stringify({ "sdp": pc.localDescription }));
        }, logError);
    }

    socket.on('message',function(data) {
        logMessage('message received');
        if (!pc) {
            start();
        }
        
        var message = JSON.parse(data);
        if (message.sdp) {
            pc.setRemoteDescription(new webrtc.RTCSessionDescription(message.sdp), function () {
                // if we received an offer, we need to answer
                if (pc.remoteDescription.type == "offer") {
                    pc.createAnswer(localDescCreated, logError);
                }
            }, logError);
        } else if (message.candidate) {
            pc.addIceCandidate(new webrtc.RTCIceCandidate(message.candidate));
        }
    });
});

server.listen(process.env.PORT || 3000, process.env.IP || "0.0.0.0", function(){
  var addr = server.address();
  console.log("Stream management server listening at", addr.address + ":" + addr.port);
});