var RTCPeerConnection     = wrtc.RTCPeerConnection;
var RTCSessionDescription = wrtc.RTCSessionDescription;
var RTCIceCandidate       = wrtc.RTCIceCandidate;

navigator.getMedia = ( navigator.getUserMedia ||
                       navigator.webkitGetUserMedia ||
                       navigator.mozGetUserMedia ||
                       navigator.msGetUserMedia);

if (!navigator.getMedia)
    console.log('Error: getUserMedia is not supported on this browser');
else {
    var socket = io.connect();

    socket.on('connected', function(){
        navigator.getMedia({video:true,audio:true}, function (source) {
            console.log('local stream created');
            media = source;
            var sourceEl = document.getElementById('source');
            sourceEl.src = window.URL.createObjectURL(media);
            start();
        }, logError);
    });

    socket.on('logmessage',function(message){
        logMessage(message);
    });

    socket.on('message',function(data){
        logMessage('message received');
        if (!pc) {
            start();
        }

        var message = JSON.parse(data);
        if (message.sdp) {
            pc.setRemoteDescription(new RTCSessionDescription(message.sdp), function () {
                // if we received an offer, we need to answer
                if (pc.remoteDescription.type == "offer") {
                    pc.createAnswer(localDescCreated, logError);
                }
            }, logError);
        } else if (message.candidate) {
            pc.addIceCandidate(new RTCIceCandidate(message.candidate));
        }
    });

    var isHost = true;
    var media  = null;

    var pc = null;
    var configuration = { "iceServers": [{ "url": "stun:stun.example.org" }] };

    var logMessage = function (message) {
        console.log(message);
    };

    var logError = function (error) {
        throw error;
    };

    var localDescCreated = function (desc) {
        pc.setLocalDescription(desc, function () {
            socket.emit('message', JSON.stringify({ "sdp": pc.localDescription }));
        }, logError);
    };

    var start = function () {
        logMessage('rtc peer connection object initializing');
        pc = new RTCPeerConnection(configuration);

        pc.onicecandidate = function (evt) {
            if (evt.candidate) {
                console.log('ice candidate sent');
                socket.emit('message', JSON.stringify({ "candidate": evt.candidate }));
            }
        };

        pc.onnegotiationneeded = function () {
            pc.createOffer(localDescCreated, logError);
        };

        pc.onaddstream = function (evt) {
            logMessage('rtc remote stream added successfully');
            var rebroadcast = document.getElementById('rebroadcast');
            rebroadcast.src = window.URL.createObjectURL(evt.stream);
        };

        if (isHost) {
            console.log('local stream added to rtc connection for broadcast');
            pc.addStream(media);
        }
    };
}