cmake_minimum_required(VERSION 4.2)
project(preparser LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Clang REQUIRED CONFIG)

execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} -print-resource-dir
    OUTPUT_VARIABLE CLANG_RESOURCE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    COMMAND_ERROR_IS_FATAL ANY
)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE MODULE_SOURCES src/*.cppm)

add_executable(preparser ${SOURCES})

target_sources(preparser
  	PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)

target_include_directories(preparser SYSTEM PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_compile_definitions(preparser PRIVATE
    ZEYTIN_CLANG_RESOURCE_DIR="${CLANG_RESOURCE_DIR}"
)

if(TARGET ZeytinWarnings)
    target_link_libraries(preparser PRIVATE ZeytinWarnings)
else()
    target_compile_options(preparser PRIVATE
        $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic>
    )
endif()

target_link_libraries(preparser PRIVATE
    clang-cpp
    LLVM
)
