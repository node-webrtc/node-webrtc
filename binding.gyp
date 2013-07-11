{
  'variables': {
    'libjingle%': 'lib/libjingle/trunk',
  },
  'targets': [
    {
      'target_name': 'node-webrtc',
      'dependencies': [],
      'variables': {
        'libjingle_out%': '<(libjingle)/out/Release/obj',
      },
      'cflags': [
        '-pthread',
        '-fno-exceptions',
        '-fno-strict-aliasing',
        '-Wall',
        '-Wno-unused-parameter',
        '-Wno-missing-field-initializers',
        '-Wextra',
        '-pipe',
        '-fno-ident',
        '-fdata-sections',
        '-ffunction-sections',
        '-fPIC'
      ],
      'defines': [
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
        '<(libjingle)',
        '<(libjingle)/third_party/webrtc',
        '<(libjingle)/third_party/webrtc/system_wrappers/interface',
        '<(libjingle)/third_party/',
      ],
      'link_settings': {
        'ldflags': [
        ],
        'libraries': [
          '-Wl,-Bstatic',
          '<(libjingle_out)/talk/libjingle_peerconnection.a',
          '<(libjingle_out)/talk/libjingle_p2p.a',
          '<(libjingle_out)/talk/libjingle_media.a',
          '<(libjingle_out)/talk/libjingle.a',
          '<(libjingle_out)/third_party/webrtc/modules/libwebrtc_utility.a',
          '<(libjingle_out)/third_party/webrtc/modules/libaudio_conference_mixer.a',
          '<(libjingle_out)/third_party/webrtc/modules/libaudio_processing.a',
          '<(libjingle_out)/third_party/webrtc/modules/libaudio_processing_sse2.a',
          '<(libjingle_out)/third_party/webrtc/modules/libaudio_coding_module.a',
          '<(libjingle_out)/third_party/webrtc/modules/libaudio_device.a',
          '<(libjingle_out)/third_party/webrtc/modules/libvideo_processing.a',
          '<(libjingle_out)/third_party/webrtc/modules/libvideo_processing_sse2.a',
          '<(libjingle_out)/third_party/webrtc/modules/libvideo_render_module.a',
          '<(libjingle_out)/third_party/webrtc/modules/libremote_bitrate_estimator.a',
          '<(libjingle_out)/third_party/webrtc/modules/librtp_rtcp.a',
          '<(libjingle_out)/third_party/webrtc/modules/libNetEq.a',
          '<(libjingle_out)/third_party/webrtc/modules/libwebrtc_opus.a',
          '<(libjingle_out)/third_party/webrtc/modules/libG711.a',
          '<(libjingle_out)/third_party/webrtc/modules/libiSAC.a',
          '<(libjingle_out)/third_party/webrtc/modules/libCNG.a',
          '<(libjingle_out)/third_party/libsrtp/libsrtp.a',
          '<(libjingle_out)/third_party/protobuf/libprotobuf_lite.a',
          '<(libjingle_out)/third_party/webrtc/system_wrappers/source/libsystem_wrappers.a',
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