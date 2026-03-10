cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0042 NEW)

macro(assign_bool var)
  if (${ARGN})
    set(${var} ON)
  else()
    set(${var} OFF)
  endif()
endmacro()

assign_bool(use_custom_libcxx "TRUE")
assign_bool(is_arm32 "y$ENV{TARGET_ARCH}" STREQUAL "yarm")
assign_bool(is_arm64 "y$ENV{TARGET_ARCH}" STREQUAL "yarm64")

if (APPLE AND ${is_arm64})
  set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE INTERNAL "" FORCE)
endif()

if (WIN32 AND ${use_custom_libcxx})
  set(CMAKE_C_COMPILER "clang-cl" CACHE STRING "" FORCE)
  set(CMAKE_CXX_COMPILER "clang-cl" CACHE STRING "" FORCE)
endif()

set(MODULE wrtc)
project(${MODULE})

include(ExternalProject)
find_package(Git REQUIRED)
find_package(Threads REQUIRED)

# depot_tools
# -----------------------------------------------------------------------------

ExternalProject_Add(
  project_depot_tools

  GIT_REPOSITORY    https://chromium.googlesource.com/chromium/tools/depot_tools.git
  GIT_TAG           4ce8ba39a3488397a2d1494f167020f21de502f3

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

# libc++
# -----------------------------------------------------------------------------

set(libwebrtc_binary_dir ${CMAKE_BINARY_DIR}/external/libwebrtc/build/${CMAKE_BUILD_TYPE})
set(libwebrtc_src_dir ${CMAKE_BINARY_DIR}/external/libwebrtc/download/src)

add_library(libc++ OBJECT IMPORTED)
add_dependencies(libc++ libwebrtc)

if(WIN32)
  set(obj_ext .obj)
else()
  set(obj_ext .o)
endif()

set(libc++_objects
  algorithm${obj_ext}
  any${obj_ext}
  atomic${obj_ext}
  barrier${obj_ext}
  bind${obj_ext}
  charconv${obj_ext}
  chrono${obj_ext}
  condition_variable${obj_ext}
  condition_variable_destructor${obj_ext}
  d2fixed${obj_ext}
  d2s${obj_ext}
  debug${obj_ext}
  exception${obj_ext}
  f2s${obj_ext}
  format${obj_ext}
  functional${obj_ext}
  future${obj_ext}
  hash${obj_ext}
  ios.instantiations${obj_ext}
  ios${obj_ext}
  iostream${obj_ext}
  locale${obj_ext}
  memory${obj_ext}
  mutex${obj_ext}
  mutex_destructor${obj_ext}
  new${obj_ext}
  optional${obj_ext}
  random${obj_ext}
  random_shuffle${obj_ext}
  regex${obj_ext}
  shared_mutex${obj_ext}
  stdexcept${obj_ext}
  string${obj_ext}
  strstream${obj_ext}
  system_error${obj_ext}
  thread${obj_ext}
  typeinfo${obj_ext}
  utility${obj_ext}
  valarray${obj_ext}
  variant${obj_ext}
  vector${obj_ext}
)

if(WIN32)
  list(APPEND libc++_objects
    legacy_debug_handler${obj_ext}
    legacy_pointer_safety${obj_ext}
    locale_win32${obj_ext}
    support${obj_ext}
    thread_win32${obj_ext}
    verbose_abort${obj_ext}
  )
endif()

list(TRANSFORM libc++_objects PREPEND ${libwebrtc_binary_dir}/obj/buildtools/third_party/libc++/libc++/)

set_property(TARGET libc++ APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(libc++ PROPERTIES IMPORTED_OBJECTS_DEBUG "${libc++_objects}" IMPORTED_OBJECTS "${libc++_objects}")

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

# NOTE(mroberts): I would like this to be INTERFACE.
#
#   https://gitlab.kitware.com/cmake/cmake/issues/15052
#
# target_include_directories(libc++abi SYSTEM INTERFACE "${libc++abi_include_dir}")

# libwebrtc
# -----------------------------------------------------------------------------

# https://chromiumdash.appspot.com/branches
# M87: branch-heads/4280
# M91: branch-heads/4472
# M92: branch-heads/4515
# M94: branch-heads/4606
# M98: branch-heads/4758
# M102: branch-heads/5005
# M106: branch-heads/5249
# M110: branch-heads/5481
# M114: branch-heads/5735
set(WEBRTC_REVISION branch-heads/5249)

file(STRINGS nix.gni NIX_GN_GEN_ARGS)
list(APPEND GN_GEN_ARGS
  rtc_build_examples=false
  rtc_use_x11=false
  rtc_enable_protobuf=false
  rtc_include_pulse_audio=false
  rtc_include_tests=false
  use_lld=false
  "${NIX_GN_GEN_ARGS}"
)

if (WIN32)
  # The escaping is very strange here
  list(APPEND GN_GEN_ARGS
    "target_cpu=\\\"$ENV{TARGET_ARCH}\\\""
  )
else()
  list(APPEND GN_GEN_ARGS
    "target_cpu=\"$ENV{TARGET_ARCH}\""
  )
endif()

if (${use_custom_libcxx})
  list(APPEND GN_GEN_ARGS
    use_custom_libcxx=true
  )
else()
  list(APPEND GN_GEN_ARGS
    use_custom_libcxx=false
  )
endif()

if (${is_arm32} OR ${is_arm64})
  list(APPEND GN_GEN_ARGS
    rtc_build_tools=true
  )
else()
  list(APPEND GN_GEN_ARGS
    rtc_build_tools=false
  )
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
    ${libc++_objects}
    ${libwebrtc_binary_dir}/obj/webrtc.lib
    ${libwebrtc_binary_dir}/obj/api/libjingle_peerconnection_api.lib
  )
else()
  set(byproducts
    ${libc++_objects}
    ${libc++abi_objects}
    ${libwebrtc_binary_dir}/obj/libwebrtc.a
    ${libwebrtc_binary_dir}/obj/api/libjingle_peerconnection_api.a
  )
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

  DOWNLOAD_COMMAND  ${CMAKE_COMMAND} -E env DEPOT_TOOLS=${depot_tools_install_dir} PLATFORM=${PLATFORM} WEBRTC_REVISION=${WEBRTC_REVISION} ${CMAKE_SOURCE_DIR}/scripts/download-webrtc.${suffix}
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

add_library(libpeerconnection STATIC IMPORTED)
add_dependencies(libpeerconnection project_libwebrtc)

if(WIN32)
  set_property(TARGET libpeerconnection PROPERTY IMPORTED_LOCATION "${libwebrtc_binary_dir}/obj/api/libjingle_peerconnection_api.lib")
else()
  set_property(TARGET libpeerconnection PROPERTY IMPORTED_LOCATION "${libwebrtc_binary_dir}/obj/api/libjingle_peerconnection_api.a")
endif()

set(libc++_include_dir
  "${libwebrtc_source_dir}/src/buildtools/third_party/libc++/trunk/include"
  "${libwebrtc_source_dir}/src/buildtools/third_party/libc++"
)
set(libc++abi_include_dir "${libwebrtc_source_dir}/src/buildtools/third_party/libc++abi/trunk/include")

# NOTE(mroberts): I would like this to be INTERFACE.
#
#   https://gitlab.kitware.com/cmake/cmake/issues/15052
#
# target_include_directories(libwebrtc SYSTEM INTERFACE
#   ${libwebrtc_source_dir}
#   ${libwebrtc_source_dir}/webrtc
#   ${libwebrtc_source_dir}/webrtc/third_party/abseil-cpp
#   ${libwebrtc_source_dir}/webrtc/third_party/libyuv/include
# )

# catch2
# -----------------------------------------------------------------------------

ExternalProject_Add(
  project_catch2

  GIT_REPOSITORY    https://github.com/catchorg/Catch2.git
  GIT_TAG           ee1450f268dfd5c13aa8670ba97e93cabaf2e15d

  PREFIX            ${CMAKE_BINARY_DIR}/external/catch2/prefix
  TMP_DIR           ${CMAKE_BINARY_DIR}/external/catch2/tmp
  STAMP_DIR         ${CMAKE_BINARY_DIR}/external/catch2/stamp
  DOWNLOAD_DIR      ${CMAKE_BINARY_DIR}/external/catch2/download
  SOURCE_DIR        ${CMAKE_BINARY_DIR}/external/catch2/src
  BINARY_DIR        ${CMAKE_BINARY_DIR}/external/catch2/build

  CONFIGURE_COMMAND ""
  BUILD_COMMAND     ""
  INSTALL_COMMAND   ""
)

ExternalProject_Get_Property(project_catch2 SOURCE_DIR)
set(catch2_install_dir ${SOURCE_DIR})

# node-webrtc
# -----------------------------------------------------------------------------

file(GLOB_RECURSE MODULE_SRC src/*.cc src/*.hh)
add_library(${MODULE} SHARED ${MODULE_SRC} ${CMAKE_JS_SRC})
set_target_properties(${MODULE} PROPERTIES PREFIX "" SUFFIX ".node")

set_property(TARGET ${MODULE} PROPERTY CXX_STANDARD 17)

# NOTE(mroberts): Workaround for
#
#   https://gitlab.kitware.com/cmake/cmake/issues/15052
#
# is to include all the header files here.
target_include_directories(${MODULE} SYSTEM PRIVATE
  ${CMAKE_JS_INC}
  ${CMAKE_SOURCE_DIR}/node_modules
  ${libwebrtc_source_dir}
  ${libwebrtc_source_dir}/webrtc
  ${libwebrtc_source_dir}/webrtc/third_party/abseil-cpp
  ${libwebrtc_source_dir}/webrtc/third_party/libyuv/include
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_dependencies(${MODULE} project_catch2)
  target_include_directories(${MODULE} SYSTEM PRIVATE
    ${catch2_install_dir}/single_include
  )
endif()

target_include_directories(${MODULE} PRIVATE
  ${CMAKE_SOURCE_DIR}
)

target_link_libraries(${MODULE} PRIVATE
  ${CMAKE_THREAD_LIBS_INIT}
  libpeerconnection
  libwebrtc
  ${CMAKE_JS_LIB}
)

target_compile_definitions(${MODULE} PRIVATE
  -DNAPI_VERSION=3
  -DUSE_BUILTIN_SW_CODECS
)

if(WIN32)
  set_property(TARGET ${MODULE}
    PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
  )

  target_link_libraries(${MODULE} PRIVATE
    dmoguids.lib
    msdmo.lib
    secur32.lib
    winmm.lib
    wmcodecdspuuid.lib
    ws2_32.lib
    Iphlpapi.lib
    delayimp.lib
  )

  if (${use_custom_libcxx})
    target_include_directories(${MODULE} SYSTEM PRIVATE
      ${libc++_include_dir}
    )

    target_link_libraries(${MODULE} PRIVATE
      libc++
    )

    target_link_options(${MODULE} PRIVATE
      /defaultlib:libcmt
      /defaultlib:libcpmt
      /nodefaultlib:libc++
      /nodefaultlib:c++
    )

    target_compile_definitions(${MODULE} PRIVATE
      -D_LIBCPP_ABI_NAMESPACE=Cr
      -D_LIBCPP_ABI_VERSION=2
      -D_LIBCPP_DISABLE_VISIBILITY_ANNOTATIONS
      -D_LIBCPP_HAS_NO_VENDOR_AVAILABILITY_ANNOTATIONS
    )
  endif()

  target_compile_definitions(${MODULE} PRIVATE
    -D_WINSOCKAPI_
    -DINCL_EXTRA_HTON_FUNCTIONS
    -DNOGDI
    -DNOMINMAX
    -DWEBRTC_WIN
    "$<$<CONFIG:Debug>:-D_HAS_ITERATOR_DEBUGGING=0>"
  )
else()
  target_link_libraries(${MODULE} PRIVATE
    -lc
    -lm
  )

  target_compile_options(${MODULE} PRIVATE
    -fno-rtti
    -fvisibility=hidden
    -Wall
    -Wextra
    -Wpedantic
    -Wno-missing-field-initializers
    -Wno-uninitialized
    -Wno-unused-local-typedefs
    -Wno-unused-variable
    "$<$<CONFIG:Debug>:-g>"
  )

  target_compile_definitions(${MODULE} PRIVATE
    -DWEBRTC_POSIX=1
    "$<$<CONFIG:Debug>:-DDEBUG>"
  )

  if (${use_custom_libcxx})
    # NOTE(mroberts): Workaround for
    #
    #   https://gitlab.kitware.com/cmake/cmake/issues/15052
    #
    # is to include all the header files here.
    target_include_directories(${MODULE} SYSTEM PRIVATE
      ${libc++_include_dir}
      ${libc++abi_include_dir}
    )

    # We statically link the same libc++ and libc++abi libraries that libwebrtc builds against.
    target_link_libraries(${MODULE} PRIVATE
      libc++
      libc++abi
      -nostdlib++ # Requires clang, but that's OK, we have nix :)
    )

    target_compile_options(${MODULE} PRIVATE
      -nostdlib++
      -nostdinc++
      -nodefaultlibs
    )

    target_compile_definitions(${MODULE} PRIVATE
      -D_LIBCPP_ABI_NAMESPACE=Cr
      -D_LIBCPP_ABI_VERSION=2
    )
  endif()

  if (APPLE)
    target_link_libraries(${MODULE} PRIVATE
      "-framework AppKit"
      "-framework AVFoundation"
    )

    target_compile_options(${MODULE} PRIVATE
      -Weverything
      -Wno-c++98-compat
      -Wno-c++98-compat-pedantic
      -Wno-covered-switch-default
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
    target_compile_definitions(${MODULE} PRIVATE
      -DWEBRTC_LINUX
    )

    target_compile_options(${MODULE} PRIVATE
      -fpermissive
    )
  endif()
endif()
