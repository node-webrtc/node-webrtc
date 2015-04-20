'use strict';

var tape = require('tape');
var SimplePeer = require('simple-peer');
var wrtc = require('..');

tape('connect once', function(t) {
    t.plan(1);
    console.log('###########################\n');
    connect(function(err) {
        t.error(err, 'connect callback');
    });
});

tape('connect loop', function(t) {
    t.plan(1);
    console.log('###########################\n');
    connectLoop(10, function(err) {
        t.error(err, 'connect callback');
    });
});

tape('connect concurrent', function(t) {
    var n = 10;
    t.plan(n);
    console.log('###########################\n');
    for (var i = 0; i < n; i += 1) {
        connect(callback);
    }

    function callback(err) {
        t.error(err, 'connect callback');
    }
});

tape('connect loop concurrent', function(t) {
    var n = 10;
    t.plan(n);
    console.log('###########################\n');
    for (var i = 0; i < n; i += 1) {
        connectLoop(10, callback);
    }

    function callback(err) {
        t.error(err, 'connect callback');
    }
});

var connIdGen = 1;

function connect(callback) {
    var connId = connIdGen;
    var connName = 'CONNECTION-' + connId;
    connIdGen += 1;
    console.log(connName, 'starting');

    // setup two peers with simple-peer
    var peer1 = new SimplePeer({
        wrtc: wrtc
    });
    var peer2 = new SimplePeer({
        wrtc: wrtc,
        initiator: true
    });

    // when peer1 has signaling data, give it to peer2, and vice versa
    peer1.on('signal', function(data) {
        console.log(connName, 'signal peer1 -> peer2:');
        console.log(' ', data);
        peer2.signal(data);
    });
    peer2.on('signal', function(data) {
        console.log(connName, 'signal peer2 -> peer1:');
        console.log(' ', data);
        peer1.signal(data);
    });

    peer1.on('error', function(err) {
        console.log(connName, 'peer1 error', err);
        callback(err);
    });
    peer2.on('error', function(err) {
        console.log(connName, 'peer2 error', err);
        callback(err);
    });

    // wait for 'connect' event
    peer1.on('connect', function() {
        console.log(connName, 'sending message');
        peer1.send('peers are for kids');
    });
    peer2.on('data', function() {
        console.log(connName, 'completed');
        callback();
    });
}

function connectLoop(count, callback) {
    if (count <= 0) {
        console.log('connect loop completed');
        return callback();
    } else {
        console.log('connect loop remain', count);
        connect(function(err) {
            if (err) {
                callback(err);
            } else {
                connectLoop(count - 1, callback);
            }
        });
    }
}
