cmake_minimum_required(VERSION 3.1)

if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/webrtc)
else()
  execute_process(COMMAND node ${CMAKE_SOURCE_DIR}/scripts/download-webrtc-libraries-and-headers.js)
endif()

set(MODULE wrtc)
include(${CMAKE_SOURCE_DIR}/NodeJS.cmake)
nodejs_init()

file(GLOB_RECURSE MODULE_SRC src/*.cc src/*.h)
add_nodejs_module(${MODULE} ${MODULE_SRC})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  target_compile_definitions(${MODULE} PRIVATE -DDEBUG)
endif()

target_include_directories(${MODULE} PRIVATE ${CMAKE_SOURCE_DIR})

find_package(Threads)
target_include_directories(${MODULE} SYSTEM PRIVATE
  ${CMAKE_SOURCE_DIR}/third_party/webrtc/include)

if(WIN32)
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")

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
    -DWEBRTC_WIN
    -DNOGDI
    -DNOMINMAX)
else()
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
    "-framework AppKit")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
  target_compile_definitions(${MODULE} PRIVATE
    -DWEBRTC_POSIX=1)
endif()
