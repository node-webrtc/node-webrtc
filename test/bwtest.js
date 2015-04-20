'use strict';

var wrtc = require('..');
var tape = require('tape');
var args = require('minimist')(process.argv.slice(2));
var SimplePeer = require('simple-peer');


module.exports = bwtest;
bwtest.tape = bwtape;


if (require.main === module) {
    main();
}


/**
 * called when running this script directly from cli
 */
function main() {
    if (typeof(args.iceConfig) === 'string') {
        // parse json of iceConfig to allow running like this:
        // node test/bwtest --iceConfig '{"ordered": false}'
        args.iceConfig = JSON.parse(args.iceConfig);
    }
    console.log('bwtest args:', args);
    bwtest(args);
}


/**
 * setup tape tests for bwtest
 */
function bwtape() {
    tape('bwtest default', function(t) {
        t.plan(1);
        bwtest({
            packetCount: 500,
        }, function(err) {
            t.error(err, 'bwtest check for error');
        });
    });

    tape('bwtest no buffering', function(t) {
        t.plan(1);
        bwtest({
            packetCount: 500,
            congestHighThreshold: 0,
            congestLowThreshold: 0
        }, function(err) {
            t.error(err, 'bwtest check for error');
        });
    });

    tape('bwtest unordered and unreliable', function(t) {
        t.plan(1);
        bwtest({
            packetCount: 500,
            iceConfig: {
                ordered: false,
                maxRetransmits: 5
            }
        }, function(err) {
            t.error(err, 'bwtest check for error');
        });
    });
}



/**
 *
 * BWTEST
 *
 * run a webrtc bandwidth test with two peers running in this process.
 *
 * @param options (optional) - see defaults inside for list of options.
 * @param callback function(err) called on success/failure.
 *
 */
function bwtest(options, callback) {

    // options is optional
    if (typeof(options) === 'function') {
        callback = options;
        options = null;
    }

    // defaults
    callback = callback || function() {};
    options = options || {};
    options.packetCount = options.packetCount || 5000;
    options.packetSize = options.packetSize || 16 * 1024;
    options.bufferedDelayMs = options.bufferedDelayMs || 5;
    options.congestHighThreshold = options.congestHighThreshold || 1024 * 1024;
    options.congestLowThreshold = options.congestLowThreshold || 256 * 1024;
    options.iceConfig = options.iceConfig || defaultIceConfig();

    var n = 0;
    var congested = 0;
    var stats = {
        startTime: 0,
        count: 0,
        bytes: 0,
        congestCount: 0,
        congestTime: 0
    };

    // setup two peers with simple-peer
    var peer1 = new SimplePeer({
        wrtc: wrtc
    });
    var peer2 = new SimplePeer({
        wrtc: wrtc,
        initiator: true,
        config: options.iceConfig
    });

    // when peer1 has signaling data, give it to peer2, and vice versa
    peer1.on('signal', peer2.signal.bind(peer2));
    peer2.on('signal', peer1.signal.bind(peer1));

    // wait for 'connect' event before using the data channel
    peer1.on('error', failure);
    peer2.on('error', failure);
    peer1.on('connect', start);
    peer2.on('data', receive);



    // functions //

    /**
     * start the test once peers are connected
     */
    function start() {
        stats.startTime = Date.now();
        send();
    }


    /**
     * send next packet and recurse
     */
    function send() {
        if (n >= options.packetCount) {
            console.log('SEND DONE!', info());
            return;
        }
        if (n % 100 === 0) {
            console.log('SENDING:', info());
        }
        if (congestion()) {
            setTimeout(send, options.bufferedDelayMs);
            return;
        }
        // TODO allocating new buffer per send as workaround to repeated Externalize()
        // after fixing the issues around that we can move back to higher scope.
        var buffer = new ArrayBuffer(options.packetSize);
        peer1.send(buffer, function(err) {
            if (err) {
                return failure(err);
            }
            n += 1;
            if (global.setImmediate) {
                global.setImmediate(send);
            } else {
                setTimeout(send, 0);
            }
        });
    }


    /**
     * callback for the receiver to update stats and finish the test
     */
    function receive(data) {
        stats.count += 1;
        stats.bytes += data.length;
        if (stats.count >= options.packetCount) {
            console.log('RECEIVE DONE!', info());
            // closing the channels so the process can exit
            peer1.destroy();
            peer2.destroy();
            callback();
        }
    }


    /**
     * failure handler
     */
    function failure(err) {

        // make sure to call the callback with error, even if anything here will throw
        setTimeout(callback.bind(null, err), 0);

        console.error('ERROR!', info(), err.stack || err);
        // closing the channels so the process can exit
        peer1.destroy();
        peer2.destroy();
    }


    /**
     * handle channel congestion using bufferedAmount
     */
    function congestion() {
        var bufferedAmount = peer1._channel && peer1._channel.bufferedAmount || 0;
        if ((bufferedAmount > options.congestHighThreshold) ||
            (congested && bufferedAmount > options.congestLowThreshold)) {
            if (!congested) {
                congested = Date.now();
            }
            stats.congestCount += 1;
        } else {
            if (congested) {
                stats.congestTime += Date.now() - congested;
            }
            congested = 0;
        }
        return congested;
    }



    /**
     * return information string on the test progress
     */
    function info() {
        var now = Date.now();
        if (congested) {
            stats.congestTime += Date.now() - congested;
            congested = now;
        }
        var took = (now - stats.startTime) / 1000;
        var bufferedAmount = peer1._channel && peer1._channel.bufferedAmount;
        return 'sent ' + n + ' received ' + stats.count + '. ' +
            'congestion #' + stats.congestCount + ' ' +
            (stats.congestTime / 1024).toFixed(3) + ' seconds. ' +
            (bufferedAmount ? 'bufferedAmount ' + bufferedAmount + '. ' : '') +
            'took ' + took.toFixed(3) + ' seconds. ' +
            'bandwidth ' + (stats.bytes / took / 1024).toFixed(0) + ' KB/s. ';
    }


    /**
     * default ice config is just here for documenting the options - see inside.
     */
    function defaultIceConfig() {
        return {
            /*
             * data channels are ordered by default - using unordered channel improves
             * performance but will likely require ordering in another app layer
             */
            // ordered: false,

            /*
             * data channels are reliable by default - using either maxRetransmits
             * or maxPacketLifeTime will change to unreliable mode
             */
            // maxRetransmits: 5,
            // maxPacketLifeTime: 3000,

            /*
             * iceServers with stun urls. not needed for this in-process test.
             */
            // iceServers: [{url: 'stun:23.21.150.121'}],
        };
    }
}
