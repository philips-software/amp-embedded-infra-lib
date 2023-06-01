function(emil_generate_artifacts)
    set(optionArgs BIN HEX LST MAP)
    set(singleArgs TARGET)
    cmake_parse_arguments(PARSE_ARGV 0 EMIL "${optionArgs}" "${singleArgs}" "")

    if (NOT TARGET ${EMIL_TARGET})
        message(FATAL_ERROR "${EMIL_TARGET} is not a target")
    endif()

    if (EMIL_MAP)
        target_link_options(${EMIL_TARGET} PRIVATE
            "LINKER:-Map=$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.map,--cref"
        )
    endif()

    if (EMIL_BIN)
        if (NOT EMIL_OBJ_COPY_TOOL)
            add_custom_command(
                TARGET ${EMIL_TARGET}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Generation of bin for target ${EMIL_TARGET} requested, but no EMIL_OBJ_COPY_TOOL is defined"
                COMMAND ${CMAKE_COMMAND} -E false
            )
        else()
            add_custom_command(
                TARGET ${EMIL_TARGET}
                POST_BUILD
                COMMAND ${EMIL_OBJ_COPY_TOOL} -O binary "$<TARGET_FILE:${EMIL_TARGET}>" "$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.bin"
            )
        endif()
    endif()

    if (EMIL_HEX)
        if (NOT EMIL_OBJ_COPY_TOOL)
            add_custom_command(
                TARGET ${EMIL_TARGET}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Generation of hex for target ${EMIL_TARGET} requested, but no EMIL_OBJ_COPY_TOOL is defined"
                COMMAND ${CMAKE_COMMAND} -E false
            )
        else()
            add_custom_command(
                TARGET ${EMIL_TARGET}
                POST_BUILD
                COMMAND ${EMIL_OBJ_COPY_TOOL} -O ihex "$<TARGET_FILE:${EMIL_TARGET}>" "$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.hex"
            )
        endif()
    endif()

    if (EMIL_LST)
        if (NOT EMIL_OBJ_DUMP_TOOL)
            add_custom_command(
                TARGET ${EMIL_TARGET}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Generation of lst for target ${EMIL_TARGET} requested, but no EMIL_OBJ_DUMP_TOOL is defined"
                COMMAND ${CMAKE_COMMAND} -E false
            )
        else()
            add_custom_command(
                TARGET ${EMIL_TARGET}
                POST_BUILD
                COMMAND ${EMIL_OBJ_DUMP_TOOL} -S "$<TARGET_FILE:${EMIL_TARGET}>" > "$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.lst"
            )
        endif()
    endif()
endfunction()
