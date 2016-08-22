{
  'targets': [
    {
      'target_name': 'action_before_build',
      'type': 'none',
      'actions': [
        {
          'action_name': 'download_webrtc_libraries_and_headers',
          'inputs': [],
          'outputs': ['third_party/webrtc'],
          'conditions': [
            ['OS=="win"', {
              'action': [
                'npm run download-webrtc-libraries-and-headers',
              ],
            }, {
              'action': [
                'npm', 'run', 'download-webrtc-libraries-and-headers',
              ],
            }],
          ],
          'message': 'Downloading WebRTC libraries and headers',
        }
      ],
    },
    {
      'target_name': 'wrtc',
      'dependencies': [
        'action_before_build',
      ],
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
        '-std=c++11',
      ],
      'xcode_settings': {
        'MACOSX_DEPLOYMENT_TARGET': '10.7',
        'OTHER_CFLAGS': [
          '-std=gnu++0x',
          '-Wno-c++0x-extensions',
          '-Wno-c++11-extensions',
          '-stdlib=libc++',
        ],
        'OTHER_LDFLAGS': [
          '-stdlib=libc++',
        ]
      },
      'defines': [
#        'TRACING',
#        'LARGEFILE_SOURCE',
#        '_FILE_OFFSET_BITS=64',
        'WEBRTC_THREAD_RR',
        'EXPAT_RELATIVE_PATH',
        'GTEST_RELATIVE_PATH',
        'JSONCPP_RELATIVE_PATH',
        'WEBRTC_RELATIVE_PATH',
#        '__STDC_FORMAT_MACROS',
#        'DYNAMIC_ANNOTATIONS_ENABLED=0',
      ],
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            '_GLIBCXX_USE_CXX11_ABI=0',
            'WEBRTC_LINUX',
            'WEBRTC_POSIX=1',
          ],
        }],
        ['OS=="mac"', {
          'defines': [
            'WEBRTC_MAC',
            'WEBRTC_IOS',
            'WEBRTC_POSIX=1',
          ],
        }],
        ['OS=="win"', {
          'defines': [
            'WEBRTC_WIN',
            'NOGDI',
            'NOMINMAX',
          ],
        }],
      ],
      'include_dirs': [
        "<!(node -e \"require('nan')\")",
        'third_party/webrtc/include',
      ],
      'link_settings': {
        'conditions': [
          ['OS=="mac"', {
            'libraries': [
              '-framework AppKit',
            ],
          }],
          ['OS=="win"', {
            'libraries': [
              '../third_party/webrtc/lib/libwebrtc.lib',
              'dmoguids.lib',
              'msdmo.lib',
              'secur32.lib',
              'winmm.lib',
              'wmcodecdspuuid.lib',
              'ws2_32.lib',
            ],
          }, {
            'libraries': [
              '../third_party/webrtc/lib/libwebrtc.a',
            ],
          }],
        ],
      },
      'sources': [
        'src/binding.cc',
        'src/create-answer-observer.cc',
        'src/create-offer-observer.cc',
        'src/datachannel.cc',
        'src/peerconnection.cc',
        'src/rtcstatsreport.cc',
        'src/rtcstatsresponse.cc',
        'src/set-local-description-observer.cc',
        'src/set-remote-description-observer.cc',
        'src/stats-observer.cc',
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
