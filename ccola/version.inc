set(CCOLA_VERSION_HEADER_TARGET generated.version_header)

macro(ccola_add_version_header)
    if (NOT ${CMAKE_PROJECT_NAME}_VERSION_MAJOR)
        set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR 0)
    endif()

    if (NOT ${CMAKE_PROJECT_NAME}_VERSION_MINOR)
        set(${CMAKE_PROJECT_NAME}_VERSION_MINOR 0)
    endif()

    if (NOT ${CMAKE_PROJECT_NAME}_VERSION_PATCH)
        set(${CMAKE_PROJECT_NAME}_VERSION_PATCH 0)
    endif()

    add_custom_target(${CCOLA_VERSION_HEADER_TARGET}
        COMMAND ${CMAKE_COMMAND} -D CCOLA_VERSION_INPUT_FILE=${CCOLA_DIR}/Version.h.in
                                 -D CCOLA_VERSION_OUTPUT_FILE=${CMAKE_BINARY_DIR}/Version.h
                                 -D CMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
                                 -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                                 -D VERSION_MAJOR=${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}
                                 -D VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR}
                                 -D VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH}
                                 -P ${CCOLA_DIR}/version_file_generator.inc
        BYPRODUCTS ${CMAKE_BINARY_DIR}/Version.h
        COMMENT "Generating version header for ${CMAKE_PROJECT_NAME}"
        VERBATIM
        SOURCES ${CMAKE_BINARY_DIR}/Version.h)
endmacro()

macro(ccola_add_version_header_dependency)
    ccola_include_directories("${CMAKE_BINARY_DIR}")
    list(APPEND CCOLA_COMPONENT_DEPENDENCIES ${CCOLA_VERSION_HEADER_TARGET})
endmacro()
