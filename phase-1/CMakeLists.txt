cmake_minimum_required(VERSION 3.12)
project(libwebrtc)

include(ExternalProject)
find_package(Git REQUIRED)
find_package(Threads REQUIRED)
find_program(iwyu_path OPTIONAL NAMES include-what-you-use iwyu)
find_program(clang_tidy_path OPTIONAL NAMES clang-tidy)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(LINUX TRUE)
endif()

set(STAGE_DIR ${CMAKE_BINARY_DIR}/stage)

# depot_tools
# -----------------------------------------------------------------------------

ExternalProject_Add(
  project_depot_tools

  GIT_REPOSITORY    https://chromium.googlesource.com/chromium/tools/depot_tools.git
  GIT_TAG           650096367cd9900e499052c49179b33b68fa1ac9

  PREFIX            ${CMAKE_BINARY_DIR}/external/depot_tools/prefix
  TMP_DIR           ${CMAKE_BINARY_DIR}/external/depot_tools/tmp
  STAMP_DIR         ${CMAKE_BINARY_DIR}/external/depot_tools/stamp
  DOWNLOAD_DIR      ${CMAKE_BINARY_DIR}/external/depot_tools/download
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/external/depot_tools/src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/external/depot_tools/build

  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
)

ExternalProject_Get_Property(project_depot_tools SOURCE_DIR)
set(depot_tools_install_dir ${SOURCE_DIR})

set(libwebrtc_binary_dir ${CMAKE_BINARY_DIR}/external/libwebrtc/build/${CMAKE_BUILD_TYPE})
set(libwebrtc_src_dir ${CMAKE_BINARY_DIR}/external/libwebrtc/download/src)

# libwebrtc
# -----------------------------------------------------------------------------

set(WEBRTC_REVISION branch-heads/4707)

list(APPEND GN_GEN_ARGS
  rtc_build_examples=false
  rtc_use_x11=false
  rtc_enable_protobuf=false
  rtc_include_pulse_audio=false
  rtc_include_tests=false
)
if ("$ENV{TARGET_ARCH}" STREQUAL "arm")
  list(APPEND GN_GEN_ARGS
    target_os="linux"
    target_cpu="arm"
    rtc_build_tools=true
    treat_warnings_as_errors=false
    fatal_linker_warnings=false
  )
elseif ("$ENV{TARGET_ARCH}" STREQUAL "arm64")
  if(NOT APPLE)
  list(APPEND GN_GEN_ARGS
    target_os="linux"
  )
  endif()
  list(APPEND GN_GEN_ARGS
    target_cpu="arm64"
    rtc_build_tools=true
    treat_warnings_as_errors=false
    fatal_linker_warnings=false
  )
else()
  list(APPEND GN_GEN_ARGS
    rtc_build_tools=false
  )
endif()

if (LINUX AND "$ENV{TARGET_ARCH}" STREQUAL "")
  message(STATUS ">>> Using system g++")
  list(APPEND GN_GEN_ARGS
    is_clang=false
    treat_warnings_as_errors=false
    use_custom_libcxx=false
  )
else()
  message(STATUS ">>> Using custom libc++")
endif()

if (WIN32)
  list(APPEND GN_GEN_ARGS is_clang=false)
endif()
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND GN_GEN_ARGS is_debug=true)
else()
  list(APPEND GN_GEN_ARGS is_debug=false)
endif()
string(REPLACE ";" " " GN_GEN_ARGS "${GN_GEN_ARGS}")

if(WIN32)
  set(suffix bat)
  set(PLATFORM windows)
else()
  set(suffix sh)
  if(APPLE)
    set(PLATFORM darwin)
  else()
    set(PLATFORM linux)
  endif()
endif()

if (WIN32)
  set(byproducts
    ${libwebrtc_binary_dir}/obj/webrtc.lib
    ${libwebrtc_binary_dir}/obj/pc/peerconnection.lib
  )
else()
  set(byproducts
    ${libc++_objects}
    ${libwebrtc_binary_dir}/obj/libwebrtc.a
    ${libwebrtc_binary_dir}/obj/pc/libpeerconnection.a
    ${libwebrtc_binary_dir}/libwebrtc-bin.tar.xz
  )
  if (NOT APPLE)
    list(APPEND byproducts ${libc++abi_objects})
  endif()
endif()

ExternalProject_Add(
  project_libwebrtc

  PREFIX            ${CMAKE_BINARY_DIR}/external/libwebrtc/prefix
  TMP_DIR           ${CMAKE_BINARY_DIR}/external/libwebrtc/tmp
  STAMP_DIR         ${CMAKE_BINARY_DIR}/external/libwebrtc/stamp
  DOWNLOAD_DIR      ${CMAKE_BINARY_DIR}/external/libwebrtc/download
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/external/libwebrtc/download/src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/external/libwebrtc/build/${CMAKE_BUILD_TYPE}

  BUILD_BYPRODUCTS  ${byproducts}

  DOWNLOAD_COMMAND  ${CMAKE_COMMAND} -E env DEPOT_TOOLS=${depot_tools_install_dir} PLATFORM=${PLATFORM} WEBRTC_REVISION=${WEBRTC_REVISION} ${CMAKE_SOURCE_DIR}/../scripts/download-webrtc.${suffix}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env BINARY_DIR=<BINARY_DIR> DEPOT_TOOLS=${depot_tools_install_dir} GN_GEN_ARGS=${GN_GEN_ARGS} SOURCE_DIR=<SOURCE_DIR> ${CMAKE_SOURCE_DIR}/../scripts/configure-webrtc.${suffix}
  BUILD_COMMAND     ${CMAKE_COMMAND} -E env CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} DEPOT_TOOLS=${depot_tools_install_dir} ${CMAKE_SOURCE_DIR}/../scripts/build-webrtc.${suffix}
  INSTALL_COMMAND   ${CMAKE_COMMAND} -E env STAGE_DIR=${STAGE_DIR} CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} ${CMAKE_SOURCE_DIR}/../scripts/pack-webrtc.${suffix}
)

add_dependencies(project_libwebrtc project_depot_tools)

ExternalProject_Get_Property(project_libwebrtc DOWNLOAD_DIR)
set(libwebrtc_source_dir "${DOWNLOAD_DIR}")

ExternalProject_Get_Property(project_libwebrtc BINARY_DIR)
set(libwebrtc_binary_dir "${BINARY_DIR}")

