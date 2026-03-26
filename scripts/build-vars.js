"use strict";

const os = require("os");
const platform = os.platform();
const arch = process.env.TARGET_ARCH ?? os.arch();

const buildFolder = `build-${platform}-${arch}`;

module.exports = {
  platform,
  arch,
  buildFolder,
};
