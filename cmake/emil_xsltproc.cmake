function(emil_fetch_xsltproc)
    if (CMAKE_HOST_WIN32)
        FetchContent_Declare(xsltproc URL https://www.zlatkovic.com/pub/libxml/libxslt-1.1.26.win32.zip)
        FetchContent_Declare(libxml2 URL https://www.zlatkovic.com/pub/libxml/libxml2-2.7.8.win32.zip)
        FetchContent_Declare(iconv URL https://www.zlatkovic.com/pub/libxml/iconv-1.9.2.win32.zip)
        FetchContent_Declare(zlib URL https://www.zlatkovic.com/pub/libxml/zlib-1.2.5.win32.zip)
        FetchContent_MakeAvailable(xsltproc libxml2 iconv zlib)

        set(EMIL_XSLTPROC_PATH "${xsltproc_SOURCE_DIR}/bin/" CACHE INTERNAL "")

        set(path "$ENV{PATH}")
        list(APPEND path "${libxml2_SOURCE_DIR}/bin" "${iconv_SOURCE_DIR}/bin" "${zlib_SOURCE_DIR}/bin")
        string(REPLACE ";" "\\;" path "${path}")
        set(EMIL_PATH_FOR_XSLTPROC "${path}" CACHE INTERNAL "")
    endif()
endfunction()

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
            cmake_path(ABSOLUTE_PATH input NORMALIZE OUTPUT_VARIABLE absolute_input)
            list(APPEND absolute_inputs "${absolute_input}")
        endif()
    endforeach()

    cmake_path(ABSOLUTE_PATH output BASE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} NORMALIZE OUTPUT_VARIABLE absolute_output)
    cmake_path(GET absolute_output PARENT_PATH absolute_directory)

    emil_fetch_xsltproc()

    find_program(xsltproc_program
        NAMES xsltproc
        HINTS ${EMIL_XSLTPROC_PATH}
    )

    if (EMIL_PATH_FOR_XSLTPROC)
        add_custom_command(
            OUTPUT ${absolute_output}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${absolute_directory}
            COMMAND ${CMAKE_COMMAND} -E env PATH=${EMIL_PATH_FOR_XSLTPROC} ${xsltproc_program} ${xslt_params} --output "${absolute_output}" ${absolute_inputs}
            DEPENDS ${absolute_inputs}
        )
    else()
        add_custom_command(
            OUTPUT ${absolute_output}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${absolute_directory}
            COMMAND ${xsltproc_program} ${xslt_params} --output "${absolute_output}" ${absolute_inputs}
            DEPENDS ${absolute_inputs}
        )
    endif()

    target_sources(${target} PRIVATE ${absolute_inputs})
endfunction()
