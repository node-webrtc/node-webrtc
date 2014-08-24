{
  'variables': {
    'libwebrtc%': 'third_party/libwebrtc/trunk',
    'libwebrtc_revision%': 'r5982'
    #'libwebrtc_revision%': 'r5385'
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
      'hard_depdency': 1,
      'type': 'none',
      'actions': [
        {
          'action_name': 'run_build_script',
          'inputs': [],
          'outputs': ['/dev/null'],
          'action': [
            'node', 'bin/build.js', '--target-arch', '<(target_arch)', '--libwebrtc-revision', '<(libwebrtc_revision)', '--configuration', '<(configuration)'
          ],
          'message': 'Run build script'
        }
      ]
    },
    {
      'target_name': 'wrtc',
      'dependencies': [ 'action_before_build' ],
      'variables': {
        'libwebrtc_out%': '<(libwebrtc)/out/<(configuration)/obj',
#         'libwebrtc_out%': '<(libwebrtc)/out/Release/obj',
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
        'DYNAMIC_ANNOTATIONS_ENABLED=0'
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
              '../<(libwebrtc_out)/webrtc/modules/libvideo_capture_module.a',
              '../<(libwebrtc_out)/webrtc/modules/remote_bitrate_estimator/librbe_components.a',
              '../<(libwebrtc_out)/webrtc/modules/libbitrate_controller.a',
              '../<(libwebrtc_out)/webrtc/modules/libremote_bitrate_estimator.a',
              '../<(libwebrtc_out)/webrtc/modules/librtp_rtcp.a',
              '../<(libwebrtc_out)/webrtc/modules/libacm2.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudio_coding_module.a',
              '../<(libwebrtc_out)/webrtc/modules/libNetEq.a',
              '../<(libwebrtc_out)/webrtc/modules/libwebrtc_i420.a',
              '../<(libwebrtc_out)/webrtc/modules/libwebrtc_opus.a',
              '../<(libwebrtc_out)/webrtc/modules/libwebrtc_video_coding.a',
              '../<(libwebrtc_out)/webrtc/modules/libG711.a',
              '../<(libwebrtc_out)/webrtc/modules/libiSAC.a',
              '../<(libwebrtc_out)/webrtc/modules/libiSACFix.a',
              '../<(libwebrtc_out)/webrtc/modules/libiLBC.a',
              '../<(libwebrtc_out)/webrtc/modules/libCNG.a',
              '../<(libwebrtc_out)/webrtc/modules/libaudioproc_debug_proto.a',
              '../<(libwebrtc_out)/webrtc/modules/libNetEq4.a',
              '../<(libwebrtc_out)/webrtc/modules/libG722.a',
              '../<(libwebrtc_out)/webrtc/modules/libPCM16B.a',
              '../<(libwebrtc_out)/webrtc/modules/libmedia_file.a',
              '../<(libwebrtc_out)/webrtc/modules/libpaced_sender.a',
              '../<(libwebrtc_out)/webrtc/modules/video_coding/codecs/vp8/libwebrtc_vp8.a',
              '../<(libwebrtc_out)/webrtc/modules/video_coding/utility/libvideo_coding_utility.a',
              '../<(libwebrtc_out)/webrtc/common_audio/libcommon_audio.a',
              '../<(libwebrtc_out)/webrtc/common_audio/libcommon_audio_sse2.a',
              '../<(libwebrtc_out)/webrtc/common_video/libcommon_video.a',
              '../<(libwebrtc_out)/webrtc/system_wrappers/source/libsystem_wrappers.a',
              '../<(libwebrtc_out)/third_party/libsrtp/libsrtp.a',
              '../<(libwebrtc_out)/third_party/opus/libopus.a',
              '../<(libwebrtc_out)/third_party/protobuf/libprotobuf_lite.a',
              '-Wl,--start-group',
              '../<(libwebrtc_out)/third_party/libvpx/libvpx.a',
              '../<(libwebrtc_out)/third_party/libvpx/libvpx_asm_offsets_vp8.a',
              '../<(libwebrtc_out)/third_party/libvpx/libvpx_intrinsics_mmx.a',
              '../<(libwebrtc_out)/third_party/libvpx/libvpx_intrinsics_sse2.a',
              '../<(libwebrtc_out)/third_party/libvpx/libvpx_intrinsics_ssse3.a',
              '-Wl,--end-group',
              '../<(libwebrtc_out)/net/third_party/nss/libcrssl.a',
              '../<(libwebrtc_out)/third_party/usrsctp/libusrsctplib.a',
              '-lssl',
              '-lnss3',
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
              '../<(libwebrtc_out)/../libbitrate_controller.a',
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
              '../<(libwebrtc_out)/../libwebrtc_utility.a',
              '../<(libwebrtc_out)/../libvideo_capture_module.a',
              '../<(libwebrtc_out)/../libbitrate_controller.a',
              '../<(libwebrtc_out)/../libwebrtc_i420.a',
              '../<(libwebrtc_out)/../libwebrtc_video_coding.a',
              '../<(libwebrtc_out)/../libiSACFix.a',
              '../<(libwebrtc_out)/../libmedia_file.a',
              '../<(libwebrtc_out)/../libpaced_sender.a',
              '../<(libwebrtc_out)/../libwebrtc_vp8.a',
              '../<(libwebrtc_out)/../libvideo_coding_utility.a',
              '../<(libwebrtc_out)/../libcommon_video.a',
              '../<(libwebrtc_out)/../libvpx.a',
              '../<(libwebrtc_out)/../libvpx_asm_offsets_vp8.a',
              '../<(libwebrtc_out)/../libvpx_intrinsics_mmx.a',
              '../<(libwebrtc_out)/../libvpx_intrinsics_sse2.a',
              '../<(libwebrtc_out)/../libvpx_intrinsics_ssse3.a',
              '../<(libwebrtc_out)/../libusrsctplib.a',
              '../<(libwebrtc_out)/../libcommon_audio_sse2.a',
              '../<(libwebrtc_out)/../libcrssl.a',
              '../<(libwebrtc_out)/../libnss_static.a',
              '../<(libwebrtc_out)/../libcrnspr.a',
              '../<(libwebrtc_out)/../libcrnss.a',
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
        'src/mediastream.cc',
        'src/mediastreamtrack.cc'
      ]
    },
    {
      "target_name": "action_after_build",
      "type": "none",
      "dependencies": [ "wrtc" ],
      "copies": [
        {
          "files": [ "<(PRODUCT_DIR)/<(module_name).node" ],
          "destination": "<(module_path)"
        }
      ]
    }
  ]
}
