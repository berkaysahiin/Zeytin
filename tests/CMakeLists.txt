add_executable(ZeytinBuildSmoke build_smoke.cpp)
target_link_libraries(ZeytinBuildSmoke PRIVATE ZeytinCommon ZeytinWarnings)

add_test(NAME zeytin.build_smoke COMMAND ZeytinBuildSmoke)

if(TARGET zeytin_generate_components)
    add_test(
        NAME zeytin.component_generation
        COMMAND ${CMAKE_COMMAND}
            -DGENERATED_CODE=${CMAKE_BINARY_DIR}/generated/components/components.rttr.cpp
            -DGENERATED_MANIFEST=${CMAKE_BINARY_DIR}/generated/components/components.manifest
            -DGENERATED_METADATA_DIR=${CMAKE_BINARY_DIR}/shared_resources/components
            -P ${CMAKE_CURRENT_SOURCE_DIR}/verify_component_generation.cmake
    )
endif()
