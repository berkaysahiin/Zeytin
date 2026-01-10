cmake_minimum_required(VERSION 4.2)
project(ZeytinCommon CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB_RECURSE MODULE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cppm)

add_library(ZeytinCommon STATIC ${SOURCES})

target_sources(ZeytinCommon
    PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)

target_include_directories(ZeytinCommon
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../engine/3rdparty
    ${CMAKE_CURRENT_LIST_DIR}/../editor/3rdparty
)
