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
      'cflags': [
	'-std=gnu++0x',
	'-Wno-c++0x-extensions',
        '-Wno-c++11-extensions'
      ],
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