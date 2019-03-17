'use strict';

const createExample = require('../../lib/browser/example');

const description = 'This example uses node-webrtc&rsquo;s RTCVideoSource and \
RTCVideoSink along with <a href="https://github.com/Automattic/node-canvas">\
node-canvas</a> to superimpose a spinning, colorful animation on top of the \
incoming video.';

const localVideo = document.createElement('video');
localVideo.autoplay = true;
localVideo.muted = true;

const remoteVideo = document.createElement('video');
remoteVideo.autoplay = true;

async function beforeAnswer(peerConnection) {
  const localStream = await window.navigator.mediaDevices.getUserMedia({
    audio: true,
    video: true
  });

  localVideo.srcObject = localStream;
  localStream.getTracks().forEach(track => peerConnection.addTrack(track, localStream));

  const remoteStream = new MediaStream(peerConnection.getReceivers().map(receiver => receiver.track));
  remoteVideo.srcObject = remoteStream;

  // NOTE(mroberts): This is a hack so that we can get a callback when the
  // RTCPeerConnection is closed. In the future, we can subscribe to
  // "connectionstatechange" events.
  const { close } = peerConnection;
  peerConnection.close = function() {
    remoteVideo.srcObject = null;

    localVideo.srcObject = null;

    localStream.getTracks().forEach(track => track.stop());

    return close.apply(this, arguments);
  };
}

createExample('video-compositing', description, { beforeAnswer });

const videos = document.createElement('div');
videos.className = 'grid';
videos.appendChild(localVideo);
videos.appendChild(remoteVideo);
document.body.appendChild(videos);
