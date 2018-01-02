'use strict';

function RTCStatsReport(internalRTCStatsReport) {
  Object.defineProperties(this, {
    timestamp: {
      value: internalRTCStatsReport.timestamp
    },
    type: {
      value: internalRTCStatsReport.type
    }
  });

  this.names = function names() {
    return internalRTCStatsReport.names();
  };

  this.stat = function stat(name) {
    return internalRTCStatsReport.stat(name);
  };
}

module.exports = RTCStatsReport;
