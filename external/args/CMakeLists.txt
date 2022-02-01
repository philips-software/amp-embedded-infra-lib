if (TARGET_ARCH_X86)
    add_library(arg_parser.args EXCLUDE_FROM_ALL INTERFACE)
    add_library(emil::arg_parser.args ALIAS arg_parser.args)

    target_include_directories(arg_parser.args INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}
    )
endif()
