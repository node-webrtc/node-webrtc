'use strict';

const { RTCPeerConnection } = require('../..');

function createRTCPeerConnections(configuration1 = {}, configuration2 = {}, options = {}) {
  options = {
    handleIce: true,
    ...options
  };
  const pc1 = new RTCPeerConnection(configuration1);
  try {
    const pc2 = new RTCPeerConnection(configuration2);
    if (options.handleIce) {
      [[pc1, pc2], [pc2, pc1]].forEach(([pcA, pcB]) => {
        pcA.addEventListener('icecandidate', ({ candidate }) => {
          if (candidate) {
            pcB.addIceCandidate(candidate);
          }
        });
      });
    }
    return [pc1, pc2];
  } catch (error) {
    pc1.close();
    throw error;
  }
}

async function doOffer(offerer, answerer) {
  const offer = await offerer.createOffer();
  await Promise.all([
    offerer.setLocalDescription(offer),
    answerer.setRemoteDescription(offer)
  ]);
}

async function doAnswer(answerer, offerer) {
  const answer = await answerer.createAnswer();
  await Promise.all([
    answerer.setLocalDescription(answer),
    offerer.setRemoteDescription(answer)
  ]);
}

async function negotiate(offerer, answerer) {
  await doOffer(offerer, answerer);
  await doAnswer(answerer, offerer);
}

async function negotiateRTCPeerConnections(options = {}) {
  options = Object.assign({}, {
    configuration: {},
    pc1Configuration: {},
    pc2Configuration: {},
    withPc1() {},
    withPc2() {},
  }, options);
  const [pc1, pc2] = createRTCPeerConnections(
    Object.assign({}, options.configuration, options.pc1Configuration),
    Object.assign({}, options.configuration, options.pc2Configuration));
  try {
    options.withPc1(pc1);
    options.withPc2(pc2);
    await negotiate(pc1, pc2);
    return [pc1, pc2];
  } catch (error) {
    pc1.close();
    pc2.close();
    throw error;
  }
}

async function getLocalTrackStats(pc, track, check = () => true) {
  let stats;
  do {
    const report = await pc.getStats();
    stats = [...report.values()]
      .find(stats => stats.type === 'track'
                  && stats.trackIdentifier === track.id
                  && !stats.remote
                  && check(stats));
  } while (!stats);
  return stats;
}

async function confirmSentFrameDimensions(source, track, pc, frame) {
  await getLocalTrackStats(pc, track, stats => {
    if (stats.frameWidth === frame.width
      && stats.frameHeight === frame.height) {
      return true;
    }
    source.onFrame(frame);
  });
}

function waitForStateChange(target, state, options = {}) {
  options = {
    event: 'statechange',
    property: 'state',
    ...options
  };
  if (target[options.property] === state) {
    return Promise.resolve();
  }
  return new Promise(resolve => {
    target.addEventListener(options.event, function listener() {
      if (target[options.property] === state) {
        target.removeEventListener(options.event, listener);
        resolve();
      }
    });
  });
}

function gatherCandidates(pc) {
  const candidates = [];
  return new Promise(resolve => {
    if (pc.iceGatheringState === 'complete') {
      resolve(candidates);
    }
    pc.addEventListener('icecandidate', ({ candidate }) => {
      if (!candidate) {
        resolve(candidates);
        return;
      }
      candidates.push(candidate);
    });
  });
}

function cleanup(pc, streams = []) {
  for (let s of streams)
    for (let t of s.getTracks())
      t.stop();
  pc.close();
}

function cleanupLater(pc, streams = []) {
  setTimeout(() => cleanup(pc, streams), 1);
}

module.exports = {
  confirmSentFrameDimensions,
  createRTCPeerConnections,
  gatherCandidates,
  getLocalTrackStats,
  doAnswer,
  doOffer,
  negotiate,
  negotiateRTCPeerConnections,
  waitForStateChange,
  cleanup,
  cleanupLater
};
