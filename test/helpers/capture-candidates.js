'use strict';

function captureCandidates(pc) {
  var candidates = [];
  return new Promise(function(resolve) {
    pc.onicecandidate = function(evt) {
      if (evt.candidate) {
        // eslint-disable-next-line no-console
        console.log(evt);
        candidates.push(evt.candidate);
      } else {
        resolve(candidates);
      }
    };
  });
}

module.exports = captureCandidates;
