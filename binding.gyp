{
  'variables': {
    'libwebrtc%': 'third_party/libwebrtc',
    #'configuration%': 'Release',
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
      'target_name': 'action_before_build',
      'variables': {
      },
      'dependencies': [],
      'hard_dependency': 1,
      'type': 'none',
      'actions': [
        {
          'action_name': 'run_build_script',
          'inputs': [],
          'outputs': ['/dev/null'],
          'action': [
            'node', 'bin/build.js', '--target-arch', '<(target_arch)', '--configuration', '$(BUILDTYPE)', '--module_path', '<(module_path)'
          ],
          'message': 'Run build script'
        }
      ]
    },
    {
      'target_name': 'wrtc',
      'dependencies': [
        'action_before_build'
      ],
      'variables': {
        'libwebrtc_out%': '<(libwebrtc)/out/$(BUILDTYPE)/obj',
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
              '../<(libwebrtc_out)/talk/libjingle_p2p.a',
              '../<(libwebrtc_out)/talk/libjingle_media.a',
              '../<(libwebrtc_out)/webrtc/p2p/librtc_p2p.a',
              '../<(libwebrtc_out)/webrtc/base/librtc_base.a',
              '../<(libwebrtc_out)/webrtc/base/librtc_base_approved.a',
#             '../<(libwebrtc_out)/chromium/src/net/third_party/nss/libcrssl.a',
              '../<(libwebrtc_out)/chromium/src/third_party/usrsctp/libusrsctplib.a',
              '../<(libwebrtc_out)/chromium/src/third_party/boringssl/libboringssl.a',
#             '-lssl',
#             '-lnss3',
            ]
          }],
          ['OS=="mac"', {
            'libraries': [
              '../<(libwebrtc_out)/../libjingle_peerconnection.a',
              '../<(libwebrtc_out)/../libjingle_p2p.a',
              '../<(libwebrtc_out)/../libjingle_media.a',
              '../<(libwebrtc_out)/../librtc_p2p.a',
              '../<(libwebrtc_out)/../librtc_base.a',
              '../<(libwebrtc_out)/../librtc_base_approved.a',
              '../<(libwebrtc_out)/../libusrsctplib.a',
              '../<(libwebrtc_out)/../libboringssl.a',
              '-framework AppKit',
              '-framework QTKit',
#             '-lssl',
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
        'src/rtcstatsreport.cc',
        'src/rtcstatsresponse.cc',
        'src/stats-observer.cc'
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "<(module_name)" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
