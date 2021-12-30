function(generate_xslt target output)

    set(inputs ${ARGV})
    list(REMOVE_AT inputs 0 1)

    set(absolute_inputs)
    set(xslt_params)
    foreach(input ${inputs})
        string(SUBSTRING "${input}" 0 2 input_start)
        if(DEFINED xslt_params OR "${input_start}" STREQUAL "--")
            if(EXISTS ${input})
                if (CMAKE_HOST_WIN32)
                    list(APPEND xslt_params "${input}")
                else()
                    list(APPEND xslt_params "'${input}'")
                endif()
            else()
                list(APPEND xslt_params ${input})
            endif()
        else()
            ccola_make_absolute_from_source(absolute_input ${input})
            list(APPEND absolute_inputs "${absolute_input}")
        endif()
    endforeach()

    ccola_make_absolute_from_intermediate(absolute_output ${output})
    get_filename_component(absolute_directory "${absolute_output}" DIRECTORY)

    find_program(xsltproc_program
        NAMES xsltproc
              xsltproc.exe
        HINTS ${CMAKE_CURRENT_LIST_DIR})

    add_custom_command(
        OUTPUT ${absolute_output}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${absolute_directory}
        COMMAND ${xsltproc_program} ${xslt_params} --output "${absolute_output}" ${absolute_inputs}
        DEPENDS ${absolute_inputs}
    )

    target_sources(${target} PRIVATE ${absolute_inputs})

    unset(absolute_inputs)
    unset(xslt_params)

endfunction()
