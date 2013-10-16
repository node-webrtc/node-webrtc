UNAME_S := $(shell uname -s)
ADDITIONAL = 
ifeq ($(UNAME_S),Darwin)
	NPM = npm_config_arch="ia32" npm
	GYP = ./node_modules/.bin/node-gyp --arch=i386
	NODE = arch --i386 node
else
	NPM = npm
	GYP = ./node_modules/.bin/node-gyp
	NODE = node
	ADDITIONAL = openssl
endif

all: deps test

prepare_env:
	mkdir -p lib/libwebrtc; cd lib; if [ ! -d "depot_tools" ]; then git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git; fi
	cd lib/libwebrtc/; ../depot_tools/gclient config http://webrtc.googlecode.com/svn/trunk/; GYP_GENERATORS=make ../depot_tools/gclient sync
	cd lib/libwebrtc/trunk/; make BUILDTYPE=Release libjingle_peerconnection libjingle_p2p libjingle_media libjingle_sound libjingle voice_engine video_engine_core webrtc_utility audio_conference_mixer audio_processing audio_processing_sse2 audio_device video_processing video_processing_sse2 video_render_module remote_bitrate_estimator rbe_components rtp_rtcp acm2 audio_coding_module NetEq webrtc_opus G711 iSAC iLBC CNG audioproc_debug_proto NetEq4 G722 PCM16B common_audio libsrtp opus protobuf_lite system_wrappers $(ADDITIONAL)

deps:
	$(NPM) install

build:
	$(GYP) configure
	./node_modules/.bin/node-gyp build -v

test:
	$(NODE) ./node_modules/.bin/_mocha --require blanket tests/*.test.js

clean:
	rm -rf lib/libwebrtc
	cd src; ../node_modules/.bin/node-gyp clean
	rm -rf node_modules

.PHONY: all prepare_env deps build test clean
