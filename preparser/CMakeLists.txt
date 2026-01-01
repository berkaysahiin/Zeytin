cmake_minimum_required(VERSION 4.2)
project(preparser CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(Clang REQUIRED CONFIG)

file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE MODULE_SOURCES src/*.cppm)

add_executable(preparser ${SOURCES})

target_sources(preparser
  	PUBLIC
    FILE_SET all_my_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)

target_include_directories(preparser PRIVATE
    ${LLVM_INCLUDE_DIRS}
    ${CLANG_INCLUDE_DIRS}
)

target_link_libraries(preparser PRIVATE
    clang-cpp
    LLVM
)
