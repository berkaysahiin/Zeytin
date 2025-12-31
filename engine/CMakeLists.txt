cmake_minimum_required(VERSION 4.2)
project(ZeytinEngine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(ENGINE_CONFIG "weditor" CACHE STRING "Build configuration: weditor or standalone")
set_property(CACHE ENGINE_CONFIG PROPERTY STRINGS weditor standalone)

file(GLOB_RECURSE SOURCES source/*.cpp)
file(GLOB_RECURSE MODULE_SOURCES source/*.cppm)

if(ENGINE_CONFIG STREQUAL "weditor")
    list(APPEND SOURCES 3rdparty/tracy/TracyClient.cpp)
endif()

add_executable(ZeytinEngine_${ENGINE_CONFIG} ${SOURCES})

target_sources(ZeytinEngine_${ENGINE_CONFIG}
  	PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)

target_include_directories(ZeytinEngine_${ENGINE_CONFIG} PRIVATE
    include
    3rdparty
    3rdparty/zmq
    3rdparty/raylib
    3rdparty/rttr
    3rdparty/tracy
)

target_link_directories(ZeytinEngine_${ENGINE_CONFIG} PRIVATE
    3rdparty/raylib/lib/linux
    3rdparty/rttr/lib/linux
    3rdparty/zmq/linux
)

target_compile_options(ZeytinEngine_${ENGINE_CONFIG} PRIVATE -w)

target_link_libraries(ZeytinEngine_${ENGINE_CONFIG} PRIVATE
    raylib
    m
    pthread
    dl
    rt
    X11
    asound
    rttr_core
)

if(ENGINE_CONFIG STREQUAL "weditor")
    target_link_libraries(ZeytinEngine_${ENGINE_CONFIG} PRIVATE zmq)
    target_compile_options(ZeytinEngine_${ENGINE_CONFIG} PRIVATE -g -O0)
    target_compile_definitions(ZeytinEngine_${ENGINE_CONFIG} PRIVATE
        DEBUG=1
        EDITOR_MODE=1
        TRACY_ENABLE=1
    )
elseif(ENGINE_CONFIG STREQUAL "standalone")
    target_compile_options(ZeytinEngine_${ENGINE_CONFIG} PRIVATE -O3)
    target_compile_definitions(ZeytinEngine_${ENGINE_CONFIG} PRIVATE
        STANDALONE=1
    )
endif()
