cmake_minimum_required(VERSION 4.2)
project(ZeytinEditor CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES
    source/*.cpp
    source/*.c
    3rdparty/imgui/*.cpp
    3rdparty/imgui_test_engine/*.cpp
    3rdparty/rlimgui/*.cpp
)
list(APPEND SOURCES 3rdparty/backward-cpp/backward.cpp)

file(GLOB_RECURSE MODULE_SOURCES source/*.cppm)

add_executable(ZeytinEditor ${SOURCES})

target_sources(ZeytinEditor
  	PUBLIC
	FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)

target_include_directories(ZeytinEditor PRIVATE
    include
    3rdparty
    3rdparty/imgui
    3rdparty/zmq
    3rdparty/rapidjson
    3rdparty/raylib
    3rdparty/imgui_test_engine
)

target_link_directories(ZeytinEditor PRIVATE
    3rdparty/raylib/lib/linux
    3rdparty/zmq/linux
)

target_compile_options(ZeytinEditor PRIVATE -w)

target_compile_definitions(ZeytinEditor PRIVATE
    DEBUG=1
    IMGUI_ENABLE_TEST_ENGINE
    IMGUI_TEST_ENGINE_ENABLE_COROUTINE_STDTHREAD_IMPL=1
)

target_link_libraries(ZeytinEditor PRIVATE
    raylib
    m
    pthread
    dl
    rt
    X11
    asound
    zmq
)
