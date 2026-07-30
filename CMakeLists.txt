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
option(ZEYTIN_WARNINGS_AS_ERRORS "Treat first-party warnings as errors" OFF)

add_library(ZeytinWarnings INTERFACE)
target_compile_options(ZeytinWarnings INTERFACE
    $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic>
    $<$<AND:$<BOOL:${ZEYTIN_WARNINGS_AS_ERRORS}>,$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,GNU>>:-Werror>
)

set(ENGINE_CONFIG "weditor" CACHE STRING "Engine configuration: weditor or standalone")
set_property(CACHE ENGINE_CONFIG PROPERTY STRINGS weditor standalone)

if(ZEYTIN_BUILD_TESTS)
    include(CTest)
endif()

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

if(ZEYTIN_BUILD_TESTS AND BUILD_TESTING)
    add_subdirectory(tests)
endif()
