module.exports = function(pc, candidates, callback) {
  var timer;

  pc.onicecandidate = function(evt) {
    if (evt.candidate) {
      console.log(evt);
      candidates.push(evt.candidate);
    }
    else {
      // TODO: trigger callback when supported
    }
  };

  timer = setInterval(function() {
    if (pc.iceGatheringState === 'complete') {
      // release the event handler reference
      pc.onicecandidate = null;
      clearInterval(timer);
      callback();
    }
  }, 100);
};