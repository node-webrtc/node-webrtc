{
  'variables': {
    'libwebrtc%': 'third_party/libwebrtc',
  },
  'conditions': [
    ['OS=="linux"', {
      'variables': {

      }
    }],
    ['OS=="mac"', {
      'variables': {

      }
    }],
  ],
  'targets': [
    {
      'target_name': 'wrtc',
      'dependencies': [],
      'variables': {
#        'libwebrtc_out%': '<(libwebrtc)/out/<(configuration)/obj',
        'libwebrtc_out%': '<(libwebrtc)/out/Release/obj',
      },
      'cflags': [
        '-pthread',
        '-fno-exceptions',
        '-fno-strict-aliasing',
        '-Wall',
        '-Wno-unused-parameter',
        '-Wno-missing-field-initializers',
        '-Wextra',
        '-Wno-unused-local-typedefs',
        '-Wno-uninitialized',
        '-Wno-unused-variable',
        '-Wno-unused-but-set-variable',
        '-pipe',
        '-fno-ident',
        '-fdata-sections',
        '-ffunction-sections',
        '-fPIC',
        '-fpermissive',
      ],
      'xcode_settings': {
        'OTHER_CFLAGS': [
          '-std=gnu++0x',
          '-Wno-c++0x-extensions',
          '-Wno-c++11-extensions',
        ]
      },
      'defines': [
#        'TRACING',
        'LARGEFILE_SOURCE',
        '_FILE_OFFSET_BITS=64',
        'WEBRTC_TARGET_PC',
        'WEBRTC_LINUX',
        'WEBRTC_THREAD_RR',
        'EXPAT_RELATIVE_PATH',
        'GTEST_RELATIVE_PATH',
        'JSONCPP_RELATIVE_PATH',
        'WEBRTC_RELATIVE_PATH',
        'POSIX',
        '__STDC_FORMAT_MACROS',
        'DYNAMIC_ANNOTATIONS_ENABLED=0',
        'WEBRTC_POSIX=1'
      ],
      'include_dirs': [
        "<!(node -p -e \"require('path').relative('.', require('path').dirname(require.resolve('nan')))\")",
        '<(libwebrtc)',
        '<(libwebrtc)/third_party/webrtc',
        '<(libwebrtc)/third_party/webrtc/system_wrappers/interface',
        '<(libwebrtc)/third_party',
      ],
      'link_settings': {
        'ldflags': [
        ],
        'conditions': [
          ['OS=="linux"', {
            'libraries': [
              '../<(libwebrtc_out)/talk/libjingle_peerconnection.a',
              '-lssl',
              '-lnss3',
            ]
          }],
          ['OS=="mac"', {
            'libraries': [
              '../<(libwebrtc_out)/../libjingle_peerconnection.a',
              '-framework AppKit',
              '-framework QTKit',
              '-lssl'
            ]
          }],
        ],
        'libraries': [
        ]
      },
      'sources': [
        'src/binding.cc',
        'src/create-offer-observer.cc',
        'src/create-answer-observer.cc',
        'src/set-local-description-observer.cc',
        'src/set-remote-description-observer.cc',
        'src/peerconnection.cc',
        'src/datachannel.cc',
      ]
    }
  ]
}
