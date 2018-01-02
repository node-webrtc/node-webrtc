'use strict';

function RTCStatsResponse(internalRTCStatsResponse) {
  this.result = function result() {
    return internalRTCStatsResponse.result();
  };
}

module.exports = RTCStatsResponse;
