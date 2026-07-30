cmake_minimum_required(VERSION 4.2)
project(ZeytinEngine CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT TARGET ZeytinCommon)
    add_subdirectory("../common" "${CMAKE_CURRENT_BINARY_DIR}/common")
endif()

set(ENGINE_CONFIG "weditor" CACHE STRING "Build configuration: weditor or standalone")
set_property(CACHE ENGINE_CONFIG PROPERTY STRINGS weditor standalone)

set(ENGINE_TARGET ZeytinEngine_${ENGINE_CONFIG})
set(RUNTIME_TARGET ZeytinRuntime_${ENGINE_CONFIG})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS source/*.cpp)
file(GLOB_RECURSE MODULE_SOURCES CONFIGURE_DEPENDS source/*.cppm)

if(ENGINE_CONFIG STREQUAL "standalone")
    list(FILTER SOURCES EXCLUDE REGEX "source/shared_input/")
    list(FILTER SOURCES EXCLUDE REGEX "source/shared_texture/")
    list(FILTER MODULE_SOURCES EXCLUDE REGEX "source/shared_input/")
    list(FILTER MODULE_SOURCES EXCLUDE REGEX "source/shared_texture/")
endif()

set(EXECUTABLE_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/source/entry_point.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/source/component/component.register.cpp"
)
list(REMOVE_ITEM SOURCES ${EXECUTABLE_SOURCES})

add_library(${RUNTIME_TARGET} STATIC ${SOURCES})
target_sources(${RUNTIME_TARGET}
    PUBLIC
    FILE_SET engine_modules TYPE CXX_MODULES FILES
    ${MODULE_SOURCES}
)
target_link_libraries(${RUNTIME_TARGET} PRIVATE ZeytinCommon)

if(TARGET preparser)
    set(GENERATED_COMPONENT_DIR "${CMAKE_BINARY_DIR}/generated/components")
    set(GENERATED_COMPONENT_CODE "${GENERATED_COMPONENT_DIR}/components.rttr.cpp")
    set(GENERATED_COMPONENT_MANIFEST "${GENERATED_COMPONENT_DIR}/components.manifest")
    set(GENERATED_COMPONENT_METADATA_DIR "${CMAKE_BINARY_DIR}/shared_resources/components")

    set(GAME_MODULE_SOURCES ${MODULE_SOURCES})
    list(FILTER GAME_MODULE_SOURCES INCLUDE REGEX "/source/game/")

    add_custom_command(
        OUTPUT
            "${GENERATED_COMPONENT_CODE}"
            "${GENERATED_COMPONENT_MANIFEST}"
        COMMAND $<TARGET_FILE:preparser>
            --compile-commands "${CMAKE_BINARY_DIR}"
            --game-source "${CMAKE_CURRENT_SOURCE_DIR}/source/game"
            --generated-code "${GENERATED_COMPONENT_CODE}"
            --component-metadata "${GENERATED_COMPONENT_METADATA_DIR}"
            --manifest "${GENERATED_COMPONENT_MANIFEST}"
        DEPENDS
            preparser
            ${RUNTIME_TARGET}
            ${GAME_MODULE_SOURCES}
        COMMENT "Generating component registration and metadata"
        VERBATIM
    )

    add_custom_target(zeytin_generate_components
        DEPENDS
            "${GENERATED_COMPONENT_CODE}"
            "${GENERATED_COMPONENT_MANIFEST}"
    )
    list(APPEND EXECUTABLE_SOURCES "${GENERATED_COMPONENT_CODE}")
endif()

add_executable(${ENGINE_TARGET} ${EXECUTABLE_SOURCES})
target_link_libraries(${ENGINE_TARGET} PRIVATE ${RUNTIME_TARGET})

if(TARGET zeytin_generate_components)
    add_dependencies(${ENGINE_TARGET} zeytin_generate_components)
endif()

foreach(target IN ITEMS ${RUNTIME_TARGET} ${ENGINE_TARGET})
    target_include_directories(${target} PRIVATE
        include
    )
    target_include_directories(${target} SYSTEM PRIVATE
        3rdparty
        3rdparty/zmq
        3rdparty/raylib
        3rdparty/rttr
    )
    if(TARGET ZeytinWarnings)
        target_link_libraries(${target} PRIVATE ZeytinWarnings)
    else()
        target_compile_options(${target} PRIVATE
            $<$<COMPILE_LANG_AND_ID:CXX,Clang,AppleClang,GNU>:-Wall;-Wextra;-Wpedantic>
        )
    endif()
endforeach()

if(WIN32)
    target_link_directories(${RUNTIME_TARGET} PUBLIC
        3rdparty/raylib/lib/windows
        3rdparty/_rttr/lib/windows
    )
    target_link_libraries(${RUNTIME_TARGET} PRIVATE
        raylib
        rttr_core
        winmm
        gdi32
        user32
        shell32
    )
else()
    target_link_directories(${RUNTIME_TARGET} PUBLIC
        3rdparty/raylib/lib/linux
        3rdparty/rttr/lib/linux
    )
    target_link_libraries(${RUNTIME_TARGET} PRIVATE
        raylib
        m
        pthread
        dl
        rt
        X11
        asound
        rttr_core
    )
endif()

if(ENGINE_CONFIG STREQUAL "weditor")
    if(WIN32)
        message(FATAL_ERROR "Windows weditor build is unsupported. Use ENGINE_CONFIG=standalone.")
    endif()

    target_link_directories(${RUNTIME_TARGET} PUBLIC 3rdparty/zmq/linux)
    target_link_libraries(${RUNTIME_TARGET} PRIVATE zmq)

    foreach(target IN ITEMS ${RUNTIME_TARGET} ${ENGINE_TARGET})
        target_compile_options(${target} PRIVATE -g -O0)
        target_compile_definitions(${target} PRIVATE
            DEBUG=1
            EDITOR_MODE=1
            TRACY_ENABLE=1
        )
    endforeach()
elseif(ENGINE_CONFIG STREQUAL "standalone")
    foreach(target IN ITEMS ${RUNTIME_TARGET} ${ENGINE_TARGET})
        target_compile_options(${target} PRIVATE -O3)
        target_compile_definitions(${target} PRIVATE STANDALONE=1)
    endforeach()
else()
    message(FATAL_ERROR "Unknown ENGINE_CONFIG: ${ENGINE_CONFIG}")
endif()
