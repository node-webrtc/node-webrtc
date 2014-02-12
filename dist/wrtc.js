(function(){
  var global = this;
  (function e(t, n, r) {
    function s(o, u) {
      if (!n[o]) {
        if (!t[o]) {
          var a = typeof require == "function" && require;
          if (!u && a) return a(o, !0);
          if (i) return i(o, !0);
          throw new Error("Cannot find module '" + o + "'")
        }
        var f = n[o] = {
          exports: {}
        };
        t[o][0].call(f.exports, function(e) {
          var n = t[o][1][e];
          return s(n ? n : e)
        }, f, f.exports, e, t, n, r)
      }
      return n[o].exports
    }
    var i = typeof require == "function" && require;
    for (var o = 0; o < r.length; o++) s(r[o]);
    return s
  })({
    1: [
      function(require, module, exports) {
        var RTCIceCandidate = global.mozRTCIceCandidate || global.webkitRTCIceCandidate || global.RTCIceCandidate;
        var RTCPeerConnection = global.mozRTCPeerConnection || global.webkitRTCPeerConnection || global.RTCPeerConnection;
        var RTCSessionDescription = global.mozRTCSessionDescription || global.webkitRTCSessionDescription || global.RTCSessionDescription;


        exports.RTCIceCandidate = RTCIceCandidate;
        exports.RTCPeerConnection = RTCPeerConnection;
        exports.RTCSessionDescription = RTCSessionDescription;

        global.wrtc = exports;
      }, {}
    ]
  }, {}, [1])

})();
