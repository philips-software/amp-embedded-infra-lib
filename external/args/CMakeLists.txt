if (TARGET_ARCH_X86)
    add_library(arg_parser.args ${EMIL_EXCLUDE_FROM_ALL} INTERFACE)

    target_include_directories(arg_parser.args INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
    )
endif()
