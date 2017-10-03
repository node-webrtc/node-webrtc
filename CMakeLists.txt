# NOTE(mroberts): This is a bit of a janky CMakeLists.txt that I put together.
# It works on macOS with node-cmake, and it's good enough for providing CLion
# integration (which drastically simplifies node-webrtc development, at least
# for me). If you want to contribute to node-webrtc, consider giving this a
# shot.
cmake_minimum_required(VERSION 3.1)

# Catch
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CMAKE_SOURCE_DIR}/third_party/catch)

if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/catch)
else()
    execute_process(COMMAND node ${CMAKE_SOURCE_DIR}/scripts/download-catch.js)
endif()

# libwebrtc.a
add_library(WebRTC INTERFACE)
target_include_directories(WebRTC INTERFACE ${CMAKE_SOURCE_DIR}/third_party/webrtc/include)
target_link_libraries(WebRTC INTERFACE ${CMAKE_SOURCE_DIR}/third_party/webrtc/lib/libwebrtc.a)

if(EXISTS ${CMAKE_SOURCE_DIR}/third_party/webrtc)
else()
    execute_process(COMMAND node ${CMAKE_SOURCE_DIR}/scripts/download-webrtc-headers-and-libraries.js)
endif()

# wrtc.node
set(MODULE wrtc)
include(${CMAKE_SOURCE_DIR}/node_modules/node-cmake/NodeJS.cmake)
nodejs_init()

file(GLOB_RECURSE MODULE_SRC src/*.cc src/*.h)
file(GLOB_RECURSE MODULE_TEST_SRC test/cpp/*.cc test/cpp/*.h)
add_nodejs_module(${MODULE} ${MODULE_SRC} ${MODULE_TEST_SRC})

execute_process(COMMAND node -e "process.stdout.write(require('../package.json').version)" OUTPUT_VARIABLE MODULE_VERSION)
execute_process(COMMAND node -e "process.stdout.write(process.versions.modules)" OUTPUT_VARIABLE MODULE_ABI)
execute_process(COMMAND node -e "process.stdout.write(process.arch)" OUTPUT_VARIABLE MODULE_ARCH)
execute_process(COMMAND node -e "process.stdout.write(process.platform)" OUTPUT_VARIABLE MODULE_PLATFORM)
set(MODULE_PATH ${CMAKE_SOURCE_DIR}/build/${MODULE}/v${MODULE_VERSION}/${CMAKE_BUILD_TYPE}/node-v${MODULE_ABI}-${MODULE_PLATFORM}-${MODULE_ARCH})
file(MAKE_DIRECTORY ${MODULE_PATH})
set_target_properties(${MODULE} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${MODULE_PATH})

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${MODULE} PUBLIC -DDEBUG)
endif()

target_include_directories(${MODULE} PUBLIC ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/test/cpp)

find_package(Threads)
target_link_libraries(${MODULE} ${CMAKE_THREAD_LIBS_INIT} WebRTC Catch)

if(APPLE)
    # add_definitions(
    target_compile_definitions(${MODULE} PUBLIC
            -DWEBRTC_MAC
            -DWEBRTC_IOS
            -DWEBRTC_POSIX=1
    )
    target_link_libraries(${MODULE}
            "-framework AppKit"
    )
elseif(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")
    target_compile_definitions(${MODULE} PUBLIC
            -DWEBRTC_POSIX=1
    )
elseif(WIN32)
    message(FATAL_ERROR "Update CMakeLists.txt to support Windows (see binding.gyp)")
endif()


set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --leak-check=full")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --track-fds=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --trace-children=yes")
set(MEMORYCHECK_COMMAND_OPTIONS "${MEMORYCHECK_COMMAND_OPTIONS} --error-exitcode=1")

include(CTest)

enable_testing()

add_test(test node ${CMAKE_SOURCE_DIR}/bin/test)
