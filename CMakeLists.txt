cmake_minimum_required(VERSION 4.2)

project(Zeytin LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ZEYTIN_BUILD_PREPARSER "Build the Zeytin component preparser" ON)
option(ZEYTIN_BUILD_ENGINE "Build the Zeytin runtime" ON)
option(ZEYTIN_BUILD_EDITOR "Build the Zeytin editor" ON)
option(ZEYTIN_BUILD_TESTS "Build Zeytin tests" ON)

set(ENGINE_CONFIG "weditor" CACHE STRING "Engine configuration: weditor or standalone")
set_property(CACHE ENGINE_CONFIG PROPERTY STRINGS weditor standalone)

add_subdirectory(common)

if(ZEYTIN_BUILD_PREPARSER)
    add_subdirectory(preparser)
endif()

if(ZEYTIN_BUILD_ENGINE)
    add_subdirectory(engine)
endif()

if(ZEYTIN_BUILD_EDITOR)
    add_subdirectory(editor)
endif()

if(ZEYTIN_BUILD_TESTS)
    include(CTest)
endif()
