function(emil_clangformat_setup prefix)
    find_program(CLANGFORMAT clang-format)

    if (CLANGFORMAT)
        foreach(clangformat_source ${ARGN})
            get_filename_component(clangformat_source ${clangformat_source} ABSOLUTE)
            list(APPEND clangformat_sources ${clangformat_source})
        endforeach()

        add_custom_target(${prefix}_clangformat
            COMMAND ${CLANGFORMAT} -style=file:${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../.clang-format -i ${clangformat_sources}
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMENT "Formatting ${prefix} with ${CLANGFORMAT} ..."
        )
    else()
        add_custom_target(${prefix}_clangformat
            COMMAND ${CMAKE_COMMAND} -E echo "clang-format not found; skipping formatting"
        )
    endif()

    if (TARGET clangformat)
        add_dependencies(clangformat ${prefix}_clangformat)
    else()
        add_custom_target(clangformat DEPENDS ${prefix}_clangformat)
    endif()
endfunction()

function(emil_clangformat_sources)
    emil_clangformat_setup(${PROJECT_NAME} ${ARGN})
endfunction()

function(emil_clangformat_target target)
    get_target_property(target_sources ${target} SOURCES)
    emil_clangformat_setup(${target} ${target_sources})
endfunction()
