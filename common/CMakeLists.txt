cmake_minimum_required(VERSION 4.2)
project(ZeytinCommon CXX)

if(NOT DEFINED CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 20)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB_RECURSE MODULE_SOURCES ${CMAKE_CURRENT_LIST_DIR}/*.cppm)

add_library(ZeytinCommon STATIC ${SOURCES})

set_target_properties(ZeytinCommon PROPERTIES
    CXX_STANDARD ${CMAKE_CXX_STANDARD}
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_sources(ZeytinCommon
    PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)

target_include_directories(ZeytinCommon SYSTEM PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/../engine/3rdparty
    ${CMAKE_CURRENT_LIST_DIR}/../editor/3rdparty
)

if(TARGET ZeytinWarnings)
    target_link_libraries(ZeytinCommon PRIVATE ZeytinWarnings)
else()
    target_compile_options(ZeytinCommon PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic>
    )
endif()
