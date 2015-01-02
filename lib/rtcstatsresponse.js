function RTCStatsResponse(internalRTCStatsResponse) {
  'use strict';

  this.result = function result() {
    return internalRTCStatsResponse.result();
  };
}

module.exports = RTCStatsResponse;
