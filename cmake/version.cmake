function(add_version_header_target target_name)
    if (NOT ${CMAKE_PROJECT_NAME}_VERSION_MAJOR)
        set(${CMAKE_PROJECT_NAME}_VERSION_MAJOR 0)
    endif()

    if (NOT ${CMAKE_PROJECT_NAME}_VERSION_MINOR)
        set(${CMAKE_PROJECT_NAME}_VERSION_MINOR 0)
    endif()

    if (NOT ${CMAKE_PROJECT_NAME}_VERSION_PATCH)
        set(${CMAKE_PROJECT_NAME}_VERSION_PATCH 0)
    endif()

    add_custom_target(${target_name}
        COMMAND ${CMAKE_COMMAND} -D VERSION_INPUT_FILE=${CMAKE_SOURCE_DIR}/cmake/version.h.in
                                 -D VERSION_OUTPUT_FILE=${CMAKE_BINARY_DIR}/version.h
                                 -D CMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
                                 -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                                 -D VERSION_MAJOR=${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}
                                 -D VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR}
                                 -D VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH}
                                 -P ${CMAKE_SOURCE_DIR}/cmake/version_file_generator.cmake
        BYPRODUCTS ${CMAKE_BINARY_DIR}/version.h
        COMMENT "Generating version header for ${CMAKE_PROJECT_NAME}"
        VERBATIM
        SOURCES ${CMAKE_BINARY_DIR}/version.h)
endfunction()

function(add_version_header_depencency target_name version_header_target)
    add_dependencies(${target_name} ${version_header_target})
    target_include_directories(${target_name} PRIVATE ${CMAKE_BINARY_DIR})
endfunction()
