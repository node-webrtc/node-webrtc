#!/usr/bin/env bash

GIT_ROOT="$(git rev-parse --show-toplevel)"
XCODE_VERSION="$(cat "${GIT_ROOT}/.xcode-version")"
xcodes install "${XCODE_VERSION}"
xcodes select "${XCODE_VERSION}"
