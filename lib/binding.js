'use strict';

try {
  module.exports = require('../build/Debug/wrtc.node');
} catch () {
  module.exports = require('../build/Release/wrtc.node');
}
