{
  'variables': {
    'mc%': 'lib/mozilla-central',
  },
  'targets': [
    {
      'target_name': 'webrtc',
      'dependencies': [],
      'variables': {
      },
      'cflags': [
        #'-pthread',
        #'-fno-exceptions',
        #'-fno-strict-aliasing',
        #'-Wall',
        #'-Wno-unused-parameter',
        #'-Wno-missing-field-initializers',
        #'-Wextra',
        #'-pipe',
        #'-fno-ident',
        #'-fdata-sections',
        #'-ffunction-sections',
        #'-fPIC'
      ],
      'defines': [
      ],
      'include_dirs': [
        #'<(libjingle)',
        #'<(libjingle)/third_party/webrtc',
        #'<(libjingle)/third_party/webrtc/system_wrappers/interface',
        #'<(libjingle)/third_party/',
      ],
      'link_settings': {
        'ldflags': [
        ],
        'libraries': [
          '-Wl,-Bstatic',
          '-Wl,-Bdynamic'
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