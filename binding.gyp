{
  'variables': {
    'mc%': 'lib/mozilla-central',
    'obj%': 'obj-x86_64-apple-darwin10.8.0'
  },
  'targets': [
    {
      'target_name': 'webrtc',
      'dependencies': [],
      'variables': {
      },
      'xcode_settings':{
        'OTHER_CFLAGS': [
                        '-std=gnu++0x',
                        '-Wno-c++0x-extensions',
                        '-Wno-c++11-extensions',
                        '-Wno-ignored-qualifiers',
                        '-Wno-invalid-offsetof'
        ]
      },
      'defines': [
      ],
      'include_dirs': [
	'<(mc)/media/webrtc/trunk/testing/gtest/include',
	'<(mc)/ipc/chromium/src',
	'<(mc)/media/mtransport',
	'<(mc)/media/mtransport/test',
	'<(mc)/media/webrtc/signaling/include',
	'<(mc)/media/webrtc/signaling/src/sipcc/core/sdp',
	'<(mc)/media/webrtc/signaling/src/sipcc/cpr/include',
	'<(mc)/media/webrtc/signaling/src/sipcc/core/includes',
	'<(mc)/media/webrtc/signaling/src/common/browser_logging',
	'<(mc)/media/webrtc/signaling/src/media',
	'<(mc)/media/webrtc/signaling/src/media-conduit',
	'<(mc)/media/webrtc/signaling/src/mediapipeline',
	'<(mc)/media/webrtc/signaling/src/sipcc/include',
	'<(mc)/media/webrtc/signaling/src/peerconnection',
	'<(mc)/media/webrtc/signaling/media-conduit',
	'<(mc)/media/webrtc/trunk/third_party/libjingle/source/',
	'<(mc)/xpcom/base/',
	'<(mc)/media/webrtc/trunk/webrtc',
	'<(mc)/ipc/chromium/src',
	'<(mc)/ipc/glue',
	'<(mc)/media/webrtc/signaling/test',
	'<(mc)/<(obj)/dist/include/nspr',
	'<(mc)/<(obj)/dist/include/nss',
	'<(mc)/<(obj)/dist/include',
	'<(mc)/<(obj)/dist/include',
      ],
      'link_settings': {
        'ldflags': [
        ],
        'libraries': [
          '../<(mc)/<(obj)/dist/lib/XUL',
          '../<(mc)/<(obj)/dist/lib/libmozalloc.dylib',
          '../<(mc)/<(obj)/dist/lib/libnspr4.a',
          '../<(mc)/<(obj)/dist/lib/libplc4.a',
          '../<(mc)/<(obj)/dist/lib/libplds4.a',
          '../<(mc)/<(obj)/dist/lib/libcrmf.a',
          '../<(mc)/<(obj)/dist/lib/libsmime3.dylib',
          '../<(mc)/<(obj)/dist/lib/libssl3.dylib',
          '../<(mc)/<(obj)/dist/lib/libnss3.dylib',
          '../<(mc)/<(obj)/dist/lib/libnssutil3.dylib',
          '../<(mc)/<(obj)/dist/lib/libxpcomglue_s.a',
#          '../<(mc)/<(obj)/dist/lib/libmtransport.a',
#          '../<(mc)/<(obj)/dist/lib/libecc.a',
#          '../<(mc)/<(obj)/dist/lib/libsipcc.a',
#          '../<(mc)/<(obj)/dist/lib/libgkmedias.a',
#          '../<(mc)/<(obj)/dist/lib/libgnksrtp_s.a',,
#	../../../../netwerk/srtp/src/libnksrtp_s.a
#	-framework AudioToolbox
#	-framework
#	AudioUnit
#	-framework
#	Carbon
#	-framework
#	CoreAudio
#	-framework
#	OpenGL
#	-framework
#	QTKit
#	-framework
#	QuartzCore
#	-framework
#	Security
#	-framework
#	SystemConfiguration
#	-framework
#	IOKit
#	-F/System/Library/PrivateFrameworks
#	-framework
#	CoreUI
#	-framework
#	CoreLocation
#	-framework
#	QuartzCore
#	-framework
#	Carbon
#	-framework
#	CoreAudio
#	-framework
#	AudioToolbox
#	-framework
#	AudioUnit
#	-framework
#	AddressBook
#	-framework
#	OpenGL
        ]
      },
      'sources': [
        'src/binding.cc',
        'src/peerconnection.cc',
      # 'src/create-session-description-observer.cc',
      # 'src/set-remote-description-observer.cc'
      ]
    }
  ]
}
