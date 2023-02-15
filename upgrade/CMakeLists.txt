add_subdirectory(boot_loader)
add_subdirectory(deploy_pack_to_external)
add_subdirectory(pack)
add_subdirectory(pack_builder)
add_subdirectory(pack_builder_instantiations)
add_subdirectory(security_key_generator)

function(emil_generate_artifacts)
    set(optionArgs BIN HEX LST MAP)
    set(singleArgs TARGET)
    cmake_parse_arguments(PARSE_ARGV 0 EMIL "${optionArgs}" "${singleArgs}" "")

    if (EMIL_BIN OR EMIL_HEX AND NOT EMIL_OBJ_COPY_TOOL)
        message(FATAL_ERROR "Generation of bin or hex target requested, but no EMIL_OBJ_COPY_TOOL is defined")
    endif()

    if (EMIL_LST AND NOT EMIL_OBJ_DUMP_TOOL)
        message(FATAL_ERROR "Generation of lst target requested, but no EMIL_OBJ_DUMP_TOOL is defined")
    endif()

    if (EMIL_MAP)
        target_link_options(${EMIL_TARGET} PRIVATE
            "LINKER:-Map=$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.map,--cref"
        )
    endif()

    if (EMIL_BIN)
        add_custom_command(
            TARGET ${EMIL_TARGET}
            POST_BUILD
            COMMAND ${EMIL_OBJ_COPY_TOOL} -O binary "$<TARGET_FILE:${EMIL_TARGET}>" "$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.bin"
        )
    endif()

    if (EMIL_HEX)
        add_custom_command(
            TARGET ${EMIL_TARGET}
            POST_BUILD
            COMMAND ${EMIL_OBJ_COPY_TOOL} -O ihex "$<TARGET_FILE:${EMIL_TARGET}>" "$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.hex"
        )
    endif()

    if (EMIL_LST)
        add_custom_command(
            TARGET ${EMIL_TARGET}
            POST_BUILD
            COMMAND ${EMIL_OBJ_DUMP_TOOL} -S "$<TARGET_FILE:${EMIL_TARGET}>" > "$<TARGET_FILE_DIR:${EMIL_TARGET}>/$<TARGET_FILE_BASE_NAME:${EMIL_TARGET}>.lst"
        )
    endif()
endfunction()
