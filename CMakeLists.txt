cmake_minimum_required(VERSION 3.12)

include(ExternalProject)
find_package(Git REQUIRED)
find_package(Threads REQUIRED)
find_program(iwyu_path OPTIONAL NAMES include-what-you-use iwyu)

# depot_tools
# -----------------------------------------------------------------------------

ExternalProject_Add(
  project_depot_tools

  GIT_REPOSITORY    https://chromium.googlesource.com/chromium/tools/depot_tools.git
  GIT_TAG           2fddb95698211db1373ebe2b16091a54eac51c9c

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

# libwebrtc
# -----------------------------------------------------------------------------

set(WEBRTC_REVISION branch-heads/70)

list(APPEND GN_GEN_ARGS
  rtc_build_examples=false
  rtc_build_tools=false
  rtc_use_x11=false
  rtc_enable_protobuf=false
  rtc_use_gtk=false
  rtc_include_pulse_audio=false
  rtc_include_tests=false
)

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

ExternalProject_Add(
  project_libwebrtc

  PREFIX            ${CMAKE_BINARY_DIR}/external/libwebrtc/prefix
  TMP_DIR           ${CMAKE_BINARY_DIR}/external/libwebrtc/tmp
  STAMP_DIR         ${CMAKE_BINARY_DIR}/external/libwebrtc/stamp
  DOWNLOAD_DIR      ${CMAKE_BINARY_DIR}/external/libwebrtc/download
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/external/libwebrtc/download/src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/external/libwebrtc/build/${CMAKE_BUILD_TYPE}

  DOWNLOAD_COMMAND  ${CMAKE_COMMAND} -E env DEPOT_TOOLS=${depot_tools_install_dir} PLATFORM=${PLATFORM} WEBRTC_REVISION=${WEBRTC_REVISION} ${CMAKE_SOURCE_DIR}/scripts/download-webrtc.${suffix}
  PATCH_COMMAND     ${CMAKE_COMMAND} -E env CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/scripts/patch-webrtc.${suffix}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env BINARY_DIR=<BINARY_DIR> DEPOT_TOOLS=${depot_tools_install_dir} GN_GEN_ARGS=${GN_GEN_ARGS} SOURCE_DIR=<SOURCE_DIR> ${CMAKE_SOURCE_DIR}/scripts/configure-webrtc.${suffix}
  BUILD_COMMAND     ${CMAKE_COMMAND} -E env CMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} DEPOT_TOOLS=${depot_tools_install_dir} ${CMAKE_SOURCE_DIR}/scripts/build-webrtc.${suffix}
  INSTALL_COMMAND   ""
)

add_dependencies(project_libwebrtc project_depot_tools)

ExternalProject_Get_Property(project_libwebrtc DOWNLOAD_DIR)
set(libwebrtc_source_dir "${DOWNLOAD_DIR}")

ExternalProject_Get_Property(project_libwebrtc BINARY_DIR)
set(libwebrtc_binary_dir "${BINARY_DIR}")

add_library(libwebrtc STATIC IMPORTED)
add_dependencies(libwebrtc project_libwebrtc)

if(WIN32)
  set_property(TARGET libwebrtc PROPERTY IMPORTED_LOCATION "${libwebrtc_binary_dir}/obj/webrtc.lib")
else()
  set_property(TARGET libwebrtc PROPERTY IMPORTED_LOCATION "${libwebrtc_binary_dir}/obj/libwebrtc.a")
endif()

# NOTE(mroberts): I would like this to be INTERFACE.
#
#   https://gitlab.kitware.com/cmake/cmake/issues/15052
#
# target_include_directories(libwebrtc SYSTEM INTERFACE
#   ${libwebrtc_source_dir}
#   ${libwebrtc_source_dir}/webrtc
#   ${libwebrtc_source_dir}/webrtc/third_party/abseil-cpp
# )

# libc++
# -----------------------------------------------------------------------------

add_library(libc++ OBJECT IMPORTED)
add_dependencies(libc++ libwebrtc)

set(libc++_objects
  algorithm.o
  any.o
  bind.o
  chrono.o
  condition_variable.o
  debug.o
  exception.o
  functional.o
  future.o
  hash.o
  ios.o
  iostream.o
  locale.o
  memory.o
  mutex.o
  new.o
  optional.o
  random.o
  regex.o
  shared_mutex.o
  stdexcept.o
  string.o
  strstream.o
  system_error.o
  thread.o
  typeinfo.o
  utility.o
  valarray.o
  variant.o
  vector.o
)
list(TRANSFORM libc++_objects PREPEND ${libwebrtc_binary_dir}/obj/buildtools/third_party/libc++/libc++/)

set_property(TARGET libc++ APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(libc++ PROPERTIES IMPORTED_OBJECTS_DEBUG "${libc++_objects}" IMPORTED_OBJECTS "${libc++_objects}")

set(libc++_include_dir "${libwebrtc_source_dir}/src/buildtools/third_party/libc++/trunk/include")

# NOTE(mroberts): I would like this to be INTERFACE.
#
#   https://gitlab.kitware.com/cmake/cmake/issues/15052
#
# target_include_directories(libc++ SYSTEM INTERFACE "${libc++_include_dir}")

# libc++abi
# -----------------------------------------------------------------------------

add_library(libc++abi OBJECT IMPORTED)
add_dependencies(libc++abi libwebrtc)

set(libc++abi_objects
  abort_message.o
  cxa_aux_runtime.o
  cxa_default_handlers.o
  cxa_demangle.o
  cxa_exception.o
  cxa_exception_storage.o
  cxa_guard.o
  cxa_handlers.o
  cxa_personality.o
  cxa_unexpected.o
  cxa_vector.o
  cxa_virtual.o
  fallback_malloc.o
  private_typeinfo.o
  stdlib_exception.o
  stdlib_stdexcept.o
  stdlib_typeinfo.o
)
list(TRANSFORM libc++abi_objects PREPEND ${libwebrtc_binary_dir}/obj/buildtools/third_party/libc++abi/libc++abi/)

set_property(TARGET libc++abi APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(libc++abi PROPERTIES IMPORTED_OBJECTS_DEBUG "${libc++abi_objects}" IMPORTED_OBJECTS "${libc++abi_objects}")

set(libc++abi_include_dir "${libwebrtc_source_dir}/src/buildtools/third_party/libc++abi/trunk/include")

# NOTE(mroberts): I would like this to be INTERFACE.
#
#   https://gitlab.kitware.com/cmake/cmake/issues/15052
#
# target_include_directories(libc++abi SYSTEM INTERFACE "${libc++abi_include_dir}")

# node-webrtc
# -----------------------------------------------------------------------------

set(MODULE wrtc)
include(${CMAKE_SOURCE_DIR}/NodeJS.cmake)
nodejs_init()

file(GLOB_RECURSE MODULE_SRC src/*.cc src/*.h)
add_nodejs_module(${MODULE} ${MODULE_SRC})

if(NOT iwyu_path STREQUAL "iwyu_path-NOTFOUND")
  set_property(TARGET ${MODULE} PROPERTY CXX_INCLUDE_WHAT_YOU_USE ${iwyu_path})
endif()

# NOTE(mroberts): Workaround for
#
#   https://gitlab.kitware.com/cmake/cmake/issues/15052
#
# is to include all the header files here.
target_include_directories(${MODULE} SYSTEM PRIVATE
  ${libwebrtc_source_dir}
  ${libwebrtc_source_dir}/webrtc
  ${libwebrtc_source_dir}/webrtc/third_party/abseil-cpp
)

target_include_directories(${MODULE} PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${MODULE} PRIVATE
  ${CMAKE_THREAD_LIBS_INIT}
  libwebrtc
)

target_compile_definitions(${MODULE} PRIVATE
  -DUSE_BUILTIN_SW_CODECS
)

if(WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /GR-")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /GR-")

  target_link_libraries(${MODULE} PRIVATE
    dmoguids.lib
    msdmo.lib
    secur32.lib
    winmm.lib
    wmcodecdspuuid.lib
    ws2_32.lib
  )

  target_compile_definitions(${MODULE} PRIVATE
    -D_WINSOCKAPI_
    -DINCL_EXTRA_HTON_FUNCTIONS
    -DNOGDI
    -DNOMINMAX
    -DWEBRTC_WIN
  )
else()
  target_compile_options(${MODULE} PRIVATE
    -fno-rtti
    -Wall
    -Wextra
    -Wno-missing-field-initializers
    -Wno-uninitialized
    -Wno-unused-local-typedefs
    -Wno-unused-variable
    -Wpedantic
  )

  target_compile_definitions(${MODULE} PRIVATE
    -DWEBRTC_POSIX=1
  )

  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(${MODULE} PRIVATE
      -g
    )

    target_compile_definitions(${MODULE} PRIVATE
      -DDEBUG
    )
  endif()

  if(APPLE)
    target_link_libraries(${MODULE} PRIVATE
      "-framework AppKit"
      "-framework AVFoundation"
    )

    target_compile_options(${MODULE} PRIVATE
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-documentation
      -Wno-exit-time-destructors
      -Wno-float-conversion
      -Wno-global-constructors
      -Wno-padded
      -Wno-shadow
      -Wno-shadow-field-in-constructor
      -Wno-shorten-64-to-32
      -Wno-sign-conversion
      -Wno-thread-safety-negative
      -Wno-unused-template
      -Wno-weak-vtables
    )

    target_compile_definitions(${MODULE} PRIVATE
      -DWEBRTC_MAC
      -DWEBRTC_IOS
    )
  else()
    # NOTE(mroberts): Workaround for
    #
    #   https://gitlab.kitware.com/cmake/cmake/issues/15052
    #
    # is to include all the header files here.
    target_include_directories(${MODULE} SYSTEM PRIVATE
      ${libc++_include_dir}
      ${libc++abi_include_dir}
    )

    # NOTE(mroberts): On Linux, we statically link the same libc++ and libc++abi
    # libraries that libwebrtc builds against.
    target_link_libraries(${MODULE} PRIVATE
      -lc
      -lm
      libc++
      libc++abi
    )

    target_compile_definitions(${MODULE} PRIVATE
      -DWEBRTC_LINUX
    )

    target_compile_options(${MODULE} PRIVATE
      -B${libwebrtc_source_dir}/src/third_party/binutils/Linux_x64/Release/bin
      -fpermissive
      -nostdinc++
      -nodefaultlibs
      --sysroot=${libwebrtc_source_dir}/src/build/linux/debian_sid_amd64-sysroot
    )
  endif()
endif()

# astyle
# -----------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND ASTYLE_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}")

  ExternalProject_Add(
    astyle
    GIT_REPOSITORY      https://github.com/Bareflank/astyle.git
    GIT_TAG             v1.2
    GIT_SHALLOW         true
    CMAKE_ARGS          ${ASTYLE_CMAKE_ARGS}
    PREFIX              ${CMAKE_BINARY_DIR}/external/astyle/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/astyle/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/astyle/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/astyle/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/astyle/src
    BINARY_DIR          ${CMAKE_BINARY_DIR}/external/astyle/build
  )

  list(APPEND ASTYLE_ARGS
    --style=google
    --lineend=linux
    --suffix=none
    --pad-oper
    --unpad-paren
    --align-pointer=type
    --align-reference=type
    --indent-preproc-define
    --indent-switches
    --indent-col1-comments
    --keep-one-line-statements
    --keep-one-line-blocks
    --pad-header
    --convert-tabs
    --min-conditional-indent=0
    --indent=spaces=2
    --indent-continuation=2
    --indent-after-parens
    --close-templates
    --add-brackets
    --break-after-logical
    ${MODULE_SRC}
  )

  if(NOT WIN32 STREQUAL "1")
    add_custom_target(
      format
      COMMAND ${CMAKE_BINARY_DIR}/bin/astyle ${ASTYLE_ARGS}
      COMMENT "running astyle"
    )
  else()
    add_custom_target(
      format
      COMMAND ${CMAKE_BINARY_DIR}/bin/astyle.exe ${ASTYLE_ARGS}
      COMMENT "running astyle"
    )
  endif()
endif()

# cppcheck
# -----------------------------------------------------------------------------

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND CPPCHECK_CMAKE_ARGS "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}")

  ExternalProject_Add(
    cppcheck
    GIT_REPOSITORY      https://github.com/danmar/cppcheck.git
    GIT_TAG             1.83
    GIT_SHALLOW         true
    CMAKE_ARGS          ${CPPCHECK_CMAKE_ARGS}
    PREFIX              ${CMAKE_BINARY_DIR}/external/cppcheck/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/cppcheck/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/cppcheck/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/cppcheck/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/src
    BINARY_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/build
  )

  list(APPEND CPPCHECK_ARGS
    --enable=warning,style,performance,portability,unusedFunction
    --std=c++11
    --verbose
    --error-exitcode=1
    --language=c++
    -i${CMAKE_SOURCE_DIR}/src/webrtc
    ${CMAKE_SOURCE_DIR}/src/*.cc
  )

  add_custom_target(
    check
    COMMAND ${CMAKE_BINARY_DIR}/bin/cppcheck ${CPPCHECK_ARGS}
    COMMENT "running cppcheck"
  )
endif()
