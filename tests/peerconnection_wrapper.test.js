/***
 *             __         _
 *      _   _ / _| ___   (_)___
 *     | | | | |_ / _ \  | / __|
 *     | |_| |  _| (_) | | \__
 *      \__,_|_|  \___(_)/ |___/
 *                     |__/
 *
 *     Proudly developed by:
 *
 *     boemianrapsodi   @gmail.com
 *     b3by.in.th3.sky  @gmail.com
 *
 *
 *     GitHub URL:
 *
 *     https://github.com/organizations/ufojs
 *
 *   This project is completely open source, and can be
 *   found on GitHub. So, if you like it and would like
 *   to collaborate, just let us know. We're totally on
 *   this sh*t, and appreciate comments and suggestions
 *   as much as we do for bacon.
 *   Do you know what else do we like? Bacon.
 *
 */

var assert = require('chai').assert;

describe('PeerconnectionWrapper', function() {
  it('should create a new object', function(done) {
    var rtcModule = require('../index');
    assert.isFunction(rtcModule.RTCPeerConnection,
                      'It contains the constructor.');
    var theObject = new rtcModule.NodeRTCPeerconnection();
    assert.isObject(theObject, 'Here is the object.');
    done();
  });

  it('should create an offer', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var test = function(desc) {
      console.log(desc);
      done();
    };
    pc.createOffer(test);
  });

  it('should throw an error if no callback was passed to createOffer', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    assert.throw(pc.createOffer, 'Not enough arguments');
    done();
  });

  it('should create an answer', function(done) {
    var rtcModule = require('../index');
    var pc1 = new rtcModule.RTCPeerConnection();
    var pc2 = new rtcModule.RTCPeerConnection();
    var onAnswer = function(desc) {
      console.log(desc);
      done();
    };
    var onRemote = function() {
      pc2.createAnswer(onAnswer);
    };
    var onOffer = function(desc) {
      pc2.setRemoteDescription(desc, onRemote);
    };
    pc1.createOffer(onOffer);
  });

  it('should call error callback when create an answer without remote', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var test = function(desc) {
      console.log(desc);
    };
    var error = function(error) {
      console.log(error);
      assert.equal('CreateAnswer can\'t be called before SetRemoteDescription.', error);
      done();
    };
    pc.createAnswer(test, error);
  });

  it('should throw an error if no callback was passed to createAnswer', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    assert.throw(pc.createAnswer, 'Not enough arguments');
    done();
  });

  it('should set the local description', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var onOfferReady = function(desc) {
      pc.setLocalDescription(desc, done);
    };
    pc.createOffer(onOfferReady);
  });

  it('should throw an error if no local description was provided', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    assert.throw(pc.setLocalDescription, 'Not enough arguments');
    done();
  });

  it('should throw an error if local description was not an object', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var test = function() {
      pc.setLocalDescription('caccolone');
    };
    assert.throw(test, 'The type of an object was incompatible with the expected type of the parameter associated to the object.');
    done();
  });

  it('should set the remote description', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var onOfferReady = function(desc) {
      pc.setRemoteDescription(desc, done);
    }
    pc.createOffer(onOfferReady);
  });

  it('should throw an error if no remote description was provided', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    assert.throw(pc.setRemoteDescription, 'Not enough arguments');
    done();
  });

  it('should throw an error if remote description was not an object', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var test = function() {
      pc.setRemoteDescription('caccolone');
    };
    assert.throw(test, 'The type of an object was incompatible with the expected type of the parameter associated to the object.');
    done();
  });

  it('should add a stream to peerconnection', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    // This adds a dumb stream. This test will be implemented better in future
    // version
    pc.addStream();
    done();
  });

  it('should throw an error if no candidate was passed', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    assert.throw(pc.addIceCandidate, 'Not enough arguments');
    done();
  });

  it('should throw an error if candidate was not an object', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var test = function() {
      pc.addIceCandidate('caccolone');
    };
    assert.throw(test, 'The type of an object was incompatible with the expected type of the parameter associated to the object.');
    done();
  });

  it('should fire onIceCandidate callback', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var fired = false;
    pc.onIceCandidate = function(event) {
      console.log(event);
      if(!fired) {
        fired = true;
        done();
      }
    };
    var onOffer = function(desc) {
      pc.setLocalDescription(desc);
    };
    pc.createOffer(onOffer);
  });

  it('should add onIceCandidate callback', function(done) {
    var rtcModule = require('../index');
    var pc = new rtcModule.RTCPeerConnection();
    var fired = false;
    pc.onIceCandidate = function(event) {
      pc.addIceCandidate(event.candidate);
      if(!fired) {
        fired = true;
        done();
      }
    };
    var onOffer = function(desc) {
      pc.setLocalDescription(desc);
    };
    pc.createOffer(onOffer);
  });


});

