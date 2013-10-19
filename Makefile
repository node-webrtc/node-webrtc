UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	NPM = npm_config_arch="ia32" npm
	GYP = ./node_modules/.bin/node-gyp --arch=i386
	NODE = arch --i386 node
else
	NPM = npm
	GYP = ./node_modules/.bin/node-gyp
	NODE = node
endif

all: deps test

prepare_env:
	mkdir -p tools; cd tools; if [ ! -d "depot_tools" ]; then git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git; fi
	mkdir -p lib/libwebrtc; cd lib/libwebrtc; ../../tools/depot_tools/gclient config http://webrtc.googlecode.com/svn/trunk/; ../../tools/depot_tools/gclient sync --force --revision r4999

build_deps:
	cd lib/libwebrtc; ninja -C out/Release peerconnection_client

deps:
	$(NPM) install

build:
	$(GYP) configure
	./node_modules/.bin/node-gyp build -v

test:
	$(NODE) ./node_modules/.bin/_mocha

docs:
	git submodule update --init
	cd third_party/doxygen; ./configure; make
	./third_party/doxygen/bin/doxygen ./confs/docs.conf
	open docs/html/index.html

clean:
	rm -rf third_party
	rm -rf docs
	cd src; ../node_modules/.bin/node-gyp clean
	rm -rf node_modules

.PHONY: prepare_env deps build test clean docs build_deps
