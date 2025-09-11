"use strict";

// TODO(jack): error handling

const childProcess = require("child_process");
const fs = require("fs");
const { platform, arch, buildFolder } = require("./build-vars.js");

// Run a fresh build
childProcess.execSync("npm run build", { stdio: "inherit" });

// Copy the resulting binary to the output folder
const inputFolder =
  platform === "win32" ? buildFolder : `${buildFolder}/Release`;
const outputFolder = `prebuilds/${platform}-${arch}`;
fs.copyFileSync(`${inputFolder}/wrtc.node`, `${outputFolder}/wrtc.node`);

if (platform === "linux") {
  // Need to patch binaries built on NixOS
  childProcess.execSync(`patchelf --remove-rpath ${outputFolder}/wrtc.node`);
}

// Copy version from main package.json to sub package.json
const mainPackageFilename = "package.json";
const subPackageFilename = `${outputFolder}/package.json`;
const mainPackage = require(`../${mainPackageFilename}`);
const subPackage = require(`../${subPackageFilename}`);

const version = mainPackage.version;
mainPackage.optionalDependencies[`@roamhq/wrtc-${platform}-${arch}`] = version;
subPackage.version = version;

fs.writeFileSync(mainPackageFilename, JSON.stringify(mainPackage, null, 2));
fs.writeFileSync(subPackageFilename, JSON.stringify(subPackage, null, 2));
