module.exports = function(pc, candidates, callback) {
  pc.onicecandidate = function(evt) {
    if (evt.candidate) {
      console.log(evt);
      candidates.push(evt.candidate);
    }
    else {
      callback();
    }
  };
};
