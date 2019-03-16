'use strict';

let audioContext = null;
let refCount = 0;

function acquireAudioContext() {
  refCount++;
  if (refCount && !audioContext) {
    audioContext = new AudioContext();
  }
  return audioContext;
}

function releaseAudioContext() {
  refCount--;
  if (!refCount && audioContext) {
    audioContext.close();
    audioContext = null;
  }
}

exports.acquireAudioContext = acquireAudioContext;
exports.releaseAudioContext = releaseAudioContext;
