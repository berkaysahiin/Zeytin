cmake_minimum_required(VERSION 4.2)
project(ZeytinEditor CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT TARGET ZeytinCommon)
    add_subdirectory("../common" "${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

file(GLOB_RECURSE SOURCES
    source/*.cpp
    source/*.c
)

file(GLOB_RECURSE THIRD_PARTY_SOURCES
    3rdparty/imgui/*.cpp
	3rdparty/imgui_test_engine/*.cpp
    3rdparty/rlimgui/*.cpp
)

list(APPEND THIRD_PARTY_SOURCES 3rdparty/backward-cpp/backward.cpp)

file(GLOB_RECURSE MODULE_SOURCES source/*.cppm)

add_executable(ZeytinEditor ${SOURCES} ${THIRD_PARTY_SOURCES})

target_sources(ZeytinEditor
  	PUBLIC
 	FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)

set_target_properties(ZeytinEditor PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_link_libraries(ZeytinEditor PRIVATE ZeytinCommon)


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

target_compile_options(ZeytinEditor PRIVATE -Wall -Wextra -Wpedantic -Werror -g -O0)

set_source_files_properties(${THIRD_PARTY_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-w"
)

set_source_files_properties(source/engine/engine_communication.cppm PROPERTIES
    COMPILE_OPTIONS "-Wno-deprecated-literal-operator"
)

set_source_files_properties(source/engine/engine_communication_impl.cpp PROPERTIES
    COMPILE_OPTIONS "-Wno-deprecated-literal-operator"
)

target_compile_definitions(ZeytinEditor PRIVATE
	NO_FONT_AWESOME
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
