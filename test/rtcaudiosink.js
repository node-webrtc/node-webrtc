"use strict";

const test = require("tape");

const { getUserMedia, RTCPeerConnection } = require("..");
const { RTCAudioSink, RTCAudioSource } = require("..").nonstandard;

test("RTCAudioSink", (t) => {
  return getUserMedia({ audio: true }).then((stream) => {
    const track = stream.getAudioTracks()[0];
    const sink = new RTCAudioSink(track);
    t.ok(!sink.stopped, "RTCAudioSink initially is not stopped");
    sink.stop();
    t.ok(sink.stopped, "RTCAudioSink is finally stopped");
    track.stop();
    t.end();
  });
});

test("RTCAudioSink should send even ondata on SIMPLE situation", (t) => {
  const source = new RTCAudioSource();
  const track = source.createTrack();
  const sink = new RTCAudioSink(track);

  sink.ondata = (data) => {
    t.ok(data, "RTCAudioSink fired ondata");
  };

  const sampleRate = 8000;
  const samples = new Int16Array(sampleRate / 100);
  source.onData({ samples, sampleRate });
  setTimeout(() => {
    sink.stop();
    track.stop();
    t.end();
  }, 150);
});

test("RTCAudioSink should send even ondata when ondata is defined in ontrack event", (t) => {
  const pcA = new RTCPeerConnection();
  const pcB = new RTCPeerConnection();
  let ondataDidFired = 0;
  let sink;

  // uncomment to debug
  // pcA.onconnectionstatechange = () => {
  //   console.log('pcA: onconnectionstatechange:', pcA.connectionState);
  // };
  // pcA.onsignalingstatechange = () => {
  //   console.log('pcA: onsignalingstatechange:', pcA.signalingState);
  // };
  // pcA.onicegatheringstatechange = (e) => {
  //   console.log('pcA: onicegatheringstatechange:', e.target.iceGatheringState);
  // };
  // pcA.ontrack = () => {
  //   console.log('pcA: onTrack');
  // };

  // pcB.onconnectionstatechange = () => {
  //   console.log('pcB: onconnectionstatechange:', pcB.connectionState);
  // };
  // pcB.onsignalingstatechange = (e) => {
  //   if (pcB.signalingState === 'stable') {
  //     console.log('pcB: onsignalingstatechange:', pcB.signalingState);
  //   }
  // };
  // pcB.onicegatheringstatechange = (e) => {
  //   console.log('pcB: onicegatheringstatechange:', e.target.iceGatheringState);
  // };

  pcB.ontrack = (e) =>
    setTimeout(() => {
      sink = new RTCAudioSink(e.track);
      sink.addEventListener("data", () => {
        ondataDidFired += 1;
      });
    }, 1);

  setupPerfectNegotiation(pcA, pcB, true);
  setupPerfectNegotiation(pcB, pcA, false);

  const source = new RTCAudioSource();
  const track = source.createTrack();
  pcA.addTrack(track);

  const sampleRate = 8000;
  const samples = new Int16Array(sampleRate / 100);
  for (let n = 0; n < samples.length; n++) {
    samples[n] = Math.random() * 0xffff;
  }

  const interval = setInterval(() => {
    source.onData({ samples, sampleRate });
  }, 10);

  setTimeout(() => {
    clearInterval(interval);
    // yes > 9 and not 10 because some random thing in eventloop and setinterval/timeout result in values to be 9||10||11
    t.ok(
      ondataDidFired >= 9,
      "RTCAudioSink should have fired 10 time in 100ms",
    );
    sink.stop();
    track.stop();
    pcA.close();
    pcB.close();
    t.end();
  }, 105);
});

test("RTCAudioSink should send ondata events when defined outside ontrack", (t) => {
  const pcA = new RTCPeerConnection();
  const pcB = new RTCPeerConnection();
  let ondataDidFired = 0;

  // uncomment to debug
  // pcA.onconnectionstatechange = () => {
  //   console.log('pcA: onconnectionstatechange:', pcA.connectionState);
  // };
  // pcA.onsignalingstatechange = () => {
  //   console.log('pcA: onsignalingstatechange:', pcA.signalingState);
  // };
  // pcA.onicegatheringstatechange = (e) => {
  //   console.log('pcA: onicegatheringstatechange:', e.target.iceGatheringState);
  // };
  // pcA.ontrack = () => {
  //   console.log('pcA: onTrack');
  // };

  // pcB.onconnectionstatechange = () => {
  //   console.log('pcB: onconnectionstatechange:', pcB.connectionState);
  // };
  // pcB.onsignalingstatechange = (e) => {
  //   if (pcB.signalingState === 'stable') {
  //     console.log('pcB: onsignalingstatechange:', pcB.signalingState);
  //   }
  // };
  // pcB.onicegatheringstatechange = (e) => {
  //   console.log('pcB: onicegatheringstatechange:', e.target.iceGatheringState);
  // };

  // pcB.ontrack = (e) =>{
  // };

  setupPerfectNegotiation(pcA, pcB, true);
  setupPerfectNegotiation(pcB, pcA, false);

  const source = new RTCAudioSource();
  const track = source.createTrack();
  pcA.addTrack(track);

  const sink = new RTCAudioSink(track);
  sink.addEventListener("data", () => {
    ondataDidFired += 1;
  });

  const sampleRate = 8000;
  const samples = new Int16Array(sampleRate / 100);
  for (let n = 0; n < samples.length; n++) {
    samples[n] = Math.random() * 0xffff;
  }
  const interval = setInterval(() => {
    source.onData({ samples, sampleRate });
  }, 10);

  setTimeout(() => {
    clearInterval(interval);
    // TODO(jack): yes >= 9 and not 10 because some random thing in eventloop and setinterval/timeout result in values to be 9||10||11
    t.ok(
      ondataDidFired >= 9,
      "RTCAudioSink should have fired 10 time in 100ms",
    );
    sink.stop();
    track.stop();
    pcA.close();
    pcB.close();
    t.end();
  }, 105);
});

/**
 * See https://developer.mozilla.org/en-US/docs/Web/API/WebRTC_API/Perfect_negotiation
 * @param {RTCPeerConnection} local
 * @param {RTCPeerConnection} remote
 * @param {boolean} polite
 */
function setupPerfectNegotiation(local, remote, polite) {
  local.onnegotiationneeded = async () => {
    try {
      await local.setLocalDescription(await local.createOffer());
      await remote.receivedescription(local.localDescription);
    } catch (err) {
      console.error(err);
    }
  };

  /**
   * @param {RTCSessionDescription} description
   */
  local.receivedescription = async (description) => {
    if (description.type === "offer") {
      if (!polite && local.signalingState !== "stable") return;
      await Promise.all([
        async () => {
          if (local.signalingState !== "stable") {
            await local.setLocalDescription({ type: "rollback" });
          }
        },
        local.setRemoteDescription(description),
      ]);
      await local.setLocalDescription(await local.createAnswer());
      await remote.receivedescription(local.localDescription);
    } else {
      await local.setRemoteDescription(description);
    }
  };
}
