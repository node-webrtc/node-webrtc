'use strict';

const {
  RTCIceCandidate,
  RTCSessionDescription
} = require('..');

function getMessage(ws, type) {
  return new Promise((resolve, reject) => {
    function onMessage({ data }) {
      try {
        const message = JSON.parse(data);
        if (message.type === type) {
          resolve(message);
        }
      } catch (error) {
        reject(error);
      } finally {
        cleanup();
      }
    }

    function onClose() {
      reject(new Error('WebSocket closed'));
      cleanup();
    }

    function cleanup() {
      ws.removeEventListener('message', onMessage);
      ws.removeEventListener('close', onClose);
    }

    ws.addEventListener('message', onMessage);
    ws.addEventListener('close', onClose);
  });
}

async function getOffer(ws) {
  const offer = await getMessage(ws, 'offer');
  return new RTCSessionDescription(offer);
}

async function getAnswer(ws) {
  const answer = await getMessage(ws, 'answer');
  return new RTCSessionDescription(answer);
}

function onCandidate(ws, callback) {
  ws.addEventListener('message', ({ data }) => {
    try {
      const message = JSON.parse(data);
      if (message.type === 'candidate') {
        const candidate = new RTCIceCandidate(message.candidate);
        callback(candidate);
        return;
      }
    } catch (error) {
      // Do nothing.
    }
  });
}

exports.getOffer = getOffer;
exports.getAnswer = getAnswer;
exports.onCandidate = onCandidate;
