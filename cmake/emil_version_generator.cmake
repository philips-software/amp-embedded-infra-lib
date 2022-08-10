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
        COMMAND ${CMAKE_COMMAND} -D VERSION_INPUT_FILE=${CMAKE_CURRENT_FUNCTION_LIST_DIR}/version.h.in
                                 -D VERSION_OUTPUT_FILE=${CMAKE_BINARY_DIR}/${target_name}/generated/Version.h
                                 -D CMAKE_PROJECT_NAME=${CMAKE_PROJECT_NAME}
                                 -D PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}
                                 -D VERSION_MAJOR=${${CMAKE_PROJECT_NAME}_VERSION_MAJOR}
                                 -D VERSION_MINOR=${${CMAKE_PROJECT_NAME}_VERSION_MINOR}
                                 -D VERSION_PATCH=${${CMAKE_PROJECT_NAME}_VERSION_PATCH}
                                 -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/version_file_generator.cmake
        BYPRODUCTS ${CMAKE_BINARY_DIR}/${target_name}/generated/Version.h
        COMMENT "Generating version header for ${CMAKE_PROJECT_NAME}"
        VERBATIM
        SOURCES ${CMAKE_BINARY_DIR}/${target_name}/generated/Version.h)

    set_target_properties(${target_name} PROPERTIES VERSION_DIR ${CMAKE_BINARY_DIR}/${target_name})
endfunction()

function(add_version_header_dependency target_name version_header_target)
    add_dependencies(${target_name} ${version_header_target})
    get_target_property(version_dir ${version_header_target} VERSION_DIR)
    target_include_directories(${target_name} PRIVATE ${version_dir})
endfunction()
