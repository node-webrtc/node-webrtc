#!/usr/bin/node
/* creates a data channel between two peer connections and sends binary messages
 *  * after ~4000 messages it will segfault
 *   * reproducible with nodejs-0.10.31 & wrtc-0.0.29
 *    */
var servers = null, options = {optional: [{DtlsSrtpKeyAgreement:true}]};
var rx, tx, tx_channel;

/* receiving peer */

rx = new (require('../').RTCPeerConnection)(servers, options);
rx.onicecandidate = function(event){
  if (event.candidate)
    tx.addIceCandidate(event.candidate);
};
rx.ondatachannel = function(event){
  var rx_channel = event.channel;
  rx_channel.onmessage = function(event){
    console.log('receieve message: ' + Uint32Array(event.data)[0]);
  };
};

/* sending peer */

var counter = 0; /* we stuff a counter in each message */

tx = new (require('../').RTCPeerConnection)(servers, options);
tx.onicecandidate = function(event){
  if (event.candidate)
    rx.addIceCandidate(event.candidate);
};

tx_channel = tx.createDataChannel("DataChannel", {reliable: false});
tx_channel.onopen = function(){
  /* when data channel is open, send lots of binary data */
  setInterval(function(){
      var raw = new ArrayBuffer(4);
      Uint32Array(raw)[0] = counter++;
      tx_channel.send(raw);
    }, 1);
};

tx.createOffer(function(desc){
  tx.setLocalDescription(desc);
  rx.setRemoteDescription(desc, null, console.log);
  rx.createAnswer(function(desc){
    rx.setLocalDescription(desc);
    tx.setRemoteDescription(desc, null, console.log);
    }, console.log);
  }, console.log);
