{
  'variables': {
    'libwebrtc%': 'lib/libwebrtc/trunk',
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
      'target_name': 'webrtc',
      'dependencies': [],
      'variables': {
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
        #'TRACING',
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
        'DYNAMIC_ANNOTATIONS_ENABLED=0'
      ],
      'include_dirs': [
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
              #'-Wl,-Bstatic',
              '../<(libwebrtc_out)/talk/libjingle_peerconnection.a',
              '../<(libwebrtc_out)/talk/libjingle_p2p.a',
              '../<(libwebrtc_out)/talk/libjingle_media.a',
              '../<(libwebrtc_out)/talk/libjingle_sound.a',
              '../<(libwebrtc_out)/talk/libjingle.a',
              '../<(libwebrtc_out)/webrtc/voice_engine/libvoice_engine.a',
              '../<(libwebrtc_out)/webrtc/video_engine/libvideo_engine_core.a',
              '../<(libwebrtc_out)/webrtc/modules/libwebrtc_utility.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudio_conference_mixer.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudio_processing.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudio_processing_sse2.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudio_device.a',
              '../<(libwebrtc_out)/webrtc/modules/libvideo_processing.a',
              '../<(libwebrtc_out)/webrtc/modules/libvideo_processing_sse2.a',
              '../<(libwebrtc_out)/webrtc/modules/libvideo_render_module.a',
              '../<(libwebrtc_out)/webrtc/modules/libremote_bitrate_estimator.a',
              '../<(libwebrtc_out)/webrtc/modules/remote_bitrate_estimator/librbe_components.a',
              '../<(libwebrtc_out)/webrtc/modules/librtp_rtcp.a',
              '../<(libwebrtc_out)/webrtc/modules/libacm2.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudio_coding_module.a',
              '../<(libwebrtc_out)/webrtc/modules/libNetEq.a',
              '../<(libwebrtc_out)/webrtc/modules/libwebrtc_opus.a',
              '../<(libwebrtc_out)/webrtc/modules/libG711.a',
              '../<(libwebrtc_out)/webrtc/modules/libiSAC.a',
              '../<(libwebrtc_out)/webrtc/modules/libiLBC.a',
              '../<(libwebrtc_out)/webrtc/modules/libCNG.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudioproc_debug_proto.a',
              '../<(libwebrtc_out)/webrtc/modules/libNetEq4.a',
              '../<(libwebrtc_out)/webrtc/modules/libG722.a',
              '../<(libwebrtc_out)/webrtc/modules/libPCM16B.a',
              '../<(libwebrtc_out)/webrtc/common_audio/libcommon_audio.a',
              '../<(libwebrtc_out)/third_party/libsrtp/libsrtp.a',
              '../<(libwebrtc_out)/third_party/opus/libopus.a',
              '../<(libwebrtc_out)/third_party/protobuf/libprotobuf_lite.a',
              '../<(libwebrtc_out)/webrtc/system_wrappers/source/libsystem_wrappers.a',
              #'../<(libwebrtc_out)/third_party/openssl/libopenssl.a',
              #'-Wl,-Bdynamic',
              '-lX11',
            ]
          }],
          ['OS=="mac"', {
            'libraries': [
              '../<(libwebrtc_out)/../libCNG.a',
              '../<(libwebrtc_out)/../libG711.a',
              '../<(libwebrtc_out)/../libG722.a',
              '../<(libwebrtc_out)/../libNetEq.a',
              '../<(libwebrtc_out)/../libNetEq4.a',
              '../<(libwebrtc_out)/../libPCM16B.a',
              '../<(libwebrtc_out)/../libacm2.a',
              '../<(libwebrtc_out)/../libaudio_coding_module.a',
              '../<(libwebrtc_out)/../libaudio_conference_mixer.a',
              '../<(libwebrtc_out)/../libaudio_device.a',
              '../<(libwebrtc_out)/../libaudio_processing.a',
              '../<(libwebrtc_out)/../libaudio_processing_sse2.a',
              '../<(libwebrtc_out)/../libaudioproc_debug_proto.a',
              '../<(libwebrtc_out)/../libcommon_audio.a',
              '../<(libwebrtc_out)/../libiLBC.a',
              '../<(libwebrtc_out)/../libiSAC.a',
              '../<(libwebrtc_out)/../libjingle.a',
              '../<(libwebrtc_out)/../libjingle_media.a',
              '../<(libwebrtc_out)/../libjingle_p2p.a',
              '../<(libwebrtc_out)/../libjingle_peerconnection.a',
              '../<(libwebrtc_out)/../libjingle_sound.a',
              '../<(libwebrtc_out)/../libopus.a',
              '../<(libwebrtc_out)/../libprotobuf_lite.a',
              '../<(libwebrtc_out)/../librbe_components.a',
              '../<(libwebrtc_out)/../libremote_bitrate_estimator.a',
              '../<(libwebrtc_out)/../librtp_rtcp.a',
              '../<(libwebrtc_out)/../libsrtp.a',
              '../<(libwebrtc_out)/../libsystem_wrappers.a',
              '../<(libwebrtc_out)/../libvideo_engine_core.a',
              '../<(libwebrtc_out)/../libvideo_processing.a',
              '../<(libwebrtc_out)/../libvideo_processing_sse2.a',
              '../<(libwebrtc_out)/../libvideo_render_module.a',
              '../<(libwebrtc_out)/../libvoice_engine.a',
              '../<(libwebrtc_out)/../libwebrtc_opus.a',
              '../<(libwebrtc_out)/../libwebrtc_utility.a'
            ]
          }],
        ],
        'libraries': [
        ]
      },
      'sources': [
        'src/binding.cc',
        'src/peerconnection.cc',
        'src/datachannel.cc'
      ]
    }
  ]
}
