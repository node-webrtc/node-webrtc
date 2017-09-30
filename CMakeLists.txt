# NOTE(mroberts): This is a bit of a janky CMakeLists.txt that I put together.
# It works on macOS with node-cmake, and it's good enough for providing CLion
# integration (which drastically simplifies node-webrtc development, at least
# for me). If you want to contribute to node-webrtc, consider giving this a
# shot.

cmake_minimum_required(VERSION 3.1)

# TODO(mroberts): We may need these to be able to copy the resulting wrtc.node
# to a place that node-pre-gyp's helper function expects.
execute_process(COMMAND npm view . version OUTPUT_VARIABLE VERSION)
execute_process(COMMAND node -e console.log(process.arch) OUTPUT_VARIABLE ARCH)
execute_process(COMMAND node -e console.log(process.platform) OUTPUT_VARIABLE PLATFORM)
execute_process(COMMAND node -e console.log(process.versions.modules) OUTPUT_VARIABLE ABI)

project(wrtc VERSION "${MODULE_VERSION}")

include(NodeJS.cmake)
nodejs_init()

file(GLOB_RECURSE PROJECT_SRC
        src/*.cc
        src/*.h
        )

include_directories(
        ${ROOT}/include
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/third_party/webrtc/include
)

add_definitions(
        -DWEBRTC_THREAD_RR
        -DEXPAT_RELATIVE_PATH
        -DGTEST_RELATIVE_PATH
        -DJSONCPP_RELATIVE_PATH
        -DWEBRTC_RELATIVE_PATH
)

# macOS
add_definitions(
        -DWEBRTC_MAC
        -DWEBRTC_IOS
        -DWEBRTC_POSIX=1
)

find_package(Threads)

link_libraries(
        ${CMAKE_SOURCE_DIR}/third_party/webrtc/lib/libwebrtc.a
        ${CMAKE_THREAD_LIBS_INIT}
)

# macOS
link_libraries(
        "-framework AppKit"
)

add_nodejs_module(${PROJECT_NAME} ${PROJECT_SRC})
