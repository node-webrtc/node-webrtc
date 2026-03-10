#!/usr/bin/env node
/* eslint no-console:0, no-process-env:0 */
"use strict";

const os = require("os");
const path = require("path");
const { spawnSync } = require("child_process");
const { platform, arch, buildFolder } = require("./build-vars.js");

const args = ["-O", buildFolder, "-a", arch];

if (process.env.DEBUG) {
  args.push(...["--debug", "--CDCMAKE_EXPORT_COMPILE_COMMANDS=1"]);
}

if (platform === "win32") {
  args.push(...["-G", "Ninja"]);
}

if (arch !== os.arch()) {
  args.push(
    `--CDCMAKE_TOOLCHAIN_FILE=toolchains/${platform}-${arch}.toolchain`,
  );
}

function main() {
  // Resolve cmake-js path before modifying PATH, since it lives in node_modules/.bin
  const cmakeJs = path.resolve(
    __dirname,
    "..",
    "node_modules",
    ".bin",
    "cmake-js",
  );

  // On Windows, remove node_modules/.bin from PATH to prevent the npm "rc"
  // package from shadowing the Windows Resource Compiler (rc.exe).
  const env = { ...process.env };
  if (platform === "win32") {
    env.PATH = env.PATH?.split(";")
      .filter((p) => !p.includes("node_modules"))
      .join(";");
  }

  console.log("Running cmake-js " + args.join(" "));
  let { status } = spawnSync(cmakeJs, ["configure", ...args], {
    shell: true,
    stdio: "inherit",
    env,
  });
  if (status) {
    throw new Error("cmake-js configure failed for wrtc");
  }

  console.log("Running cmake-js build");
  status = spawnSync(cmakeJs, ["build", ...args], {
    shell: true,
    stdio: "inherit",
    env,
  }).status;
  if (status) {
    throw new Error("cmake-js build failed for wrtc");
  }

  console.log("Built wrtc");
}

module.exports = main;

if (require.main === module) {
  main();
}
