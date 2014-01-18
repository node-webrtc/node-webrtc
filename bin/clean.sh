#!/bin/sh

rm -rf build
rm -rf docs
rm -rf third_party
cd src; ../node_modules/.bin/node-gyp clean
rm -rf node_modules
