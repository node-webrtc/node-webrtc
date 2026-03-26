"use strict";

const test = require("tape");

const { getUserMedia } = require("..");

test("getSettings", (t) => {
  return getUserMedia({
    audio: true,
    video: true,
  }).then((media) => {
    let audioTrack = media.getAudioTracks()[0];
    // TODO(jack): assert some properties about these maybe? perhaps when I
    // switch away from getUserMedia
    t.assert(audioTrack.getSettings());
    let videoTrack = media.getVideoTracks()[0];
    // TODO(jack): assert some properties about these maybe? perhaps when I
    // switch away from getUserMedia
    t.assert(videoTrack.getSettings());
    audioTrack.stop();
    videoTrack.stop();
    t.end();
  });
});
