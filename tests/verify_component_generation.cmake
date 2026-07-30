foreach(required_variable IN ITEMS
    GENERATED_CODE
    GENERATED_MANIFEST
    GENERATED_METADATA_DIR
)
    if(NOT DEFINED ${required_variable})
        message(FATAL_ERROR "Missing required variable: ${required_variable}")
    endif()
endforeach()

if(NOT EXISTS "${GENERATED_CODE}")
    message(FATAL_ERROR "Generated registration source not found: ${GENERATED_CODE}")
endif()

if(NOT EXISTS "${GENERATED_MANIFEST}")
    message(FATAL_ERROR "Generated component manifest not found: ${GENERATED_MANIFEST}")
endif()

file(READ "${GENERATED_CODE}" generated_code)
if(NOT generated_code MATCHES "RTTR_REGISTRATION")
    message(FATAL_ERROR "Generated source does not contain RTTR registration")
endif()

file(STRINGS "${GENERATED_MANIFEST}" metadata_files)
list(LENGTH metadata_files metadata_count)
if(metadata_count EQUAL 0)
    message(FATAL_ERROR "Generated component manifest is empty")
endif()

foreach(metadata_file IN LISTS metadata_files)
    if(NOT EXISTS "${metadata_file}")
        message(FATAL_ERROR "Manifest references missing metadata: ${metadata_file}")
    endif()
endforeach()

set(transform_metadata "${GENERATED_METADATA_DIR}/CTransform.component")
if(NOT EXISTS "${transform_metadata}")
    message(FATAL_ERROR "Expected transform metadata not found: ${transform_metadata}")
endif()
