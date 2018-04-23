cmake_minimum_required(VERSION 3.6)

include(ExternalProject)
find_package(Git REQUIRED)

if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/webrtc)
else()
  execute_process(COMMAND node ${CMAKE_SOURCE_DIR}/scripts/download-webrtc-libraries-and-headers.js)
endif()

set(MODULE wrtc)
include(${CMAKE_SOURCE_DIR}/NodeJS.cmake)
nodejs_init()

file(GLOB_RECURSE MODULE_SRC src/*.cc src/*.h)
add_nodejs_module(${MODULE} ${MODULE_SRC})

file(GLOB_RECURSE MODULE_SRC_WITHOUT_WEBRTC src/*.cc src/*.h)
list(REMOVE_ITEM MODULE_SRC_WITHOUT_WEBRTC
  ${CMAKE_SOURCE_DIR}/src/webrtc/physicalsocketserver.h
  ${CMAKE_SOURCE_DIR}/src/webrtc/physicalsocketserver.cc)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${MODULE} PRIVATE -DDEBUG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

target_include_directories(${MODULE} PRIVATE ${CMAKE_SOURCE_DIR})

find_package(Threads)
target_include_directories(${MODULE} SYSTEM PRIVATE
  ${CMAKE_SOURCE_DIR}/third_party/webrtc/include)

if(WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /GR-")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /GR-")

  target_link_libraries(${MODULE} PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_SOURCE_DIR}/third_party/webrtc/lib/webrtc.lib
    dmoguids.lib
    msdmo.lib
    secur32.lib
    winmm.lib
    wmcodecdspuuid.lib
    ws2_32.lib)

  target_compile_definitions(${MODULE} PRIVATE
    -DINCL_EXTRA_HTON_FUNCTIONS
    -DWEBRTC_WIN
    -DNOGDI
    -DNOMINMAX)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti")

  target_link_libraries(${MODULE} PRIVATE
    ${CMAKE_THREAD_LIBS_INIT}
    ${CMAKE_SOURCE_DIR}/third_party/webrtc/lib/libwebrtc.a)

  target_compile_options(${MODULE} PUBLIC
    # -Werror
    -Wall
    -Wpedantic
    -Wno-missing-field-initializers
    -Wextra
    -Wno-unused-local-typedefs
    -Wno-uninitialized
    -Wno-unused-variable)
endif()

if(APPLE)
  target_compile_definitions(${MODULE} PRIVATE
    -DWEBRTC_MAC
    -DWEBRTC_IOS
    -DWEBRTC_POSIX=1)

  target_link_libraries(${MODULE} PRIVATE
    "-framework AppKit"
    "-framework AVFoundation")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
  target_compile_definitions(${MODULE} PRIVATE
    -D_GLIBCXX_USE_CXX11_ABI=0
    -DWEBRTC_LINUX
    -DWEBRTC_POSIX=1)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  list(APPEND ASTYLE_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}")

  ExternalProject_Add(
    astyle
    GIT_REPOSITORY      https://github.com/Bareflank/astyle.git
    GIT_TAG             v1.2
    GIT_SHALLOW         1
    CMAKE_ARGS          ${ASTYLE_CMAKE_ARGS}
    PREFIX              ${CMAKE_BINARY_DIR}/external/astyle/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/astyle/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/astyle/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/astyle/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/astyle/src
    BINARY_DIR          ${CMAKE_BINARY_DIR}/external/astyle/build)

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
    ${MODULE_SRC_WITHOUT_WEBRTC})

  if(NOT WIN32 STREQUAL "1")
    add_custom_target(
      format
      COMMAND ${CMAKE_BINARY_DIR}/bin/astyle ${ASTYLE_ARGS}
      COMMENT "running astyle")
  else()
    add_custom_target(
      format
      COMMAND ${CMAKE_BINARY_DIR}/bin/astyle.exe ${ASTYLE_ARGS}
      COMMENT "running astyle")
  endif()

  list(APPEND CPPCHECK_CMAKE_ARGS
    "-DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}")

  ExternalProject_Add(
    cppcheck
    GIT_REPOSITORY      https://github.com/danmar/cppcheck.git
    GIT_TAG             1.83
    GIT_SHALLOW         1
    CMAKE_ARGS          ${CPPCHECK_CMAKE_ARGS}
    PREFIX              ${CMAKE_BINARY_DIR}/external/cppcheck/prefix
    TMP_DIR             ${CMAKE_BINARY_DIR}/external/cppcheck/tmp
    STAMP_DIR           ${CMAKE_BINARY_DIR}/external/cppcheck/stamp
    DOWNLOAD_DIR        ${CMAKE_BINARY_DIR}/external/cppcheck/download
    SOURCE_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/src
    BINARY_DIR          ${CMAKE_BINARY_DIR}/external/cppcheck/build)

  list(APPEND CPPCHECK_ARGS
    --enable=warning,style,performance,portability,unusedFunction
    --std=c++11
    --verbose
    --error-exitcode=1
    --language=c++
    -i${CMAKE_SOURCE_DIR}/src/webrtc
    ${CMAKE_SOURCE_DIR}/src/*.cc)

  add_custom_target(
    check
    COMMAND ${CMAKE_BINARY_DIR}/bin/cppcheck ${CPPCHECK_ARGS}
    COMMENT "running cppcheck")
endif()
